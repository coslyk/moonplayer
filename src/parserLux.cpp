#include "parserLux.h"
#include "accessManager.h"
#include "dialogs.h"
#include "platform/paths.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSettings>

ParserLux ParserLux::s_instance;

ParserLux::ParserLux(QObject *parent) : ParserBase(parent)
{
    // Connect
    connect(&m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &ParserLux::parseOutput);
    connect(&m_process, &QProcess::errorOccurred, [&](){ showErrorDialog(m_process.errorString()); });
}


ParserLux::~ParserLux()
{
    // Kill lux process
    if (m_process.state() == QProcess::Running)
    {
        m_process.kill();
        m_process.waitForFinished();
    }
}


void ParserLux::runParser(const QUrl &url)
{
    // Check if another task is running
    if (m_process.state() == QProcess::Running)
    {
        Q_ASSERT(Dialogs::instance() != nullptr);
        Dialogs::instance()->messageDialog(tr("Error"), tr("Another file is being parsed."));
        return;
    }

    // Get and apply proxy settings
    QSettings settings;
    NetworkAccessManager::ProxyType proxyType = (NetworkAccessManager::ProxyType) settings.value(QStringLiteral("network/proxy_type")).toInt();
    QString proxy = settings.value(QStringLiteral("network/proxy")).toString();

    if (!proxy.isEmpty() && proxyType == NetworkAccessManager::HTTP_PROXY)
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("HTTP_PROXY"), QStringLiteral("http://%1/").arg(proxy));
        m_process.setProcessEnvironment(env);
    }
    else if (!proxy.isEmpty() && proxyType == NetworkAccessManager::SOCKS5_PROXY)
    {
        QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
        env.insert(QStringLiteral("HTTP_PROXY"), QStringLiteral("socks5://%1/").arg(proxy));
        m_process.setProcessEnvironment(env);
    }

    // Set user-agent
    QStringList args;
    args << QStringLiteral("-j") << QStringLiteral("-p") << QStringLiteral("-u") << QStringLiteral(DEFAULT_UA);

    args << url.toString();
    m_process.start(userResourcesPath() + QStringLiteral("/lux"), args, QProcess::ReadOnly);
}


void ParserLux::parseOutput()
{
    QByteArray output = m_process.readAllStandardOutput();

#ifdef Q_OS_WIN
    // Convert to UTF-8 on Windows
    output = QString::fromLocal8Bit(output).toUtf8();
#endif

    // Parse JSON
    QJsonParseError json_error;
    QJsonDocument document = QJsonDocument::fromJson(output, &json_error);

    if (json_error.error != QJsonParseError::NoError)
    {
        showErrorDialog(QString::fromUtf8(m_process.readAllStandardError()));
        return;
    }

    // Get episode list
    QJsonArray episodes = document.array();
    if (episodes.size() == 0)
    {
        Dialogs::instance()->messageDialog(tr("Error"), tr("The given url has no video item."));
    }
    else if (episodes.size() == 1)
    {
        parseEpisode(episodes[0].toObject());
    }
    else
    {
        QStringList titles;
        for (auto episode : episodes)
        {
            titles << episode.toObject()[QStringLiteral("title")].toString();
            Dialogs::instance()->selectionDialog(tr("Select episode"), titles, [=](int index) {
                parseEpisode(episodes[index].toObject());
            });
        }
    }
}


void ParserLux::parseEpisode(QJsonObject episode)
{
    // Check error
    if (!episode[QStringLiteral("err")].isNull())
    {
        showErrorDialog(QStringLiteral("Lux Error: ") + episode[QStringLiteral("err")].toString());
        return;
    }

    // Get title
    result.title = episode[QStringLiteral("title")].toString();

    // Get danmaku
    if (!episode[QStringLiteral("caption")].isNull())
    {
        QJsonObject caption = episode[QStringLiteral("caption")].toObject();
        if (!caption[QStringLiteral("danmaku")].isNull())
        {
            result.danmaku_url = caption[QStringLiteral("danmaku")].toObject()[QStringLiteral("url")].toString();
        }
    }

    // get all available streams
    QJsonObject streams = episode[QStringLiteral("streams")].toObject();
    for (auto i = streams.constBegin(); i != streams.constEnd(); i++)
    {
        QJsonObject item = i.value().toObject();

        // Basic stream infos
        Stream stream;
        stream.container = item[QStringLiteral("ext")].toString();
        stream.is_dash = item[QStringLiteral("NeedMux")].toBool();
        stream.referer = episode[QStringLiteral("url")].toString();
        stream.seekable = true;

        // Write urls list
        QJsonArray parts = item[QStringLiteral("parts")].toArray();
        if (parts.count() == 0)   // this stream is not available, skip it
        {
            continue;
        }

        for (const auto& part : parts)
        {
            stream.urls << QUrl(part.toObject()[QStringLiteral("url")].toString());
        }

        // Add stream to list
        result.stream_types << item[QStringLiteral("quality")].toString();
        result.streams << stream;
    }
    finishParsing();
}
