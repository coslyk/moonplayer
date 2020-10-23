#include "parserYkdl.h"
#include "accessManager.h"
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTextCodec>
#include "platform/paths.h"
#include "dialogs.h"
#include "playlistModel.h"

ParserYkdl ParserYkdl::s_instance;

ParserYkdl::ParserYkdl(QObject *parent) : ParserBase(parent)
{
    connect(&m_process, SIGNAL(finished(int)),this, SLOT(parseOutput()));
    connect(&m_process, &QProcess::errorOccurred, [&](){ showErrorDialog(m_process.errorString()); });
}

ParserYkdl::~ParserYkdl()
{
    if (m_process.state() == QProcess::Running)
    {
        m_process.kill();
        m_process.waitForFinished();
    }
}

bool ParserYkdl::isSupported ( const QUrl& url )
{
#ifdef Q_OS_WIN
    QString ykdlPath = userResourcesPath() + QStringLiteral("/ykdl-moonplayer.exe");
#else
    QString ykdlPath = userResourcesPath() + QStringLiteral("/ykdl-moonplayer");
#endif
    if (QFile::exists(ykdlPath))
    {
        QProcess process;
        QStringList args;
        args << QStringLiteral("--check-support") << url.toString();
        process.start(ykdlPath, args);
        process.waitForStarted(-1);
        process.waitForFinished(-1);
        return process.readAllStandardOutput().simplified() == "Url is supported.";
    } else {
        return false;
    }
}


void ParserYkdl::runParser(const QUrl &url)
{
    if (m_process.state() == QProcess::Running)
    {
        QMessageBox::warning(nullptr, tr("Error"), tr("Another file is being parsed."));
        return;
    }

    QSettings settings;
    NetworkAccessManager::ProxyType proxyType = (NetworkAccessManager::ProxyType) settings.value(QStringLiteral("network/proxy_type")).toInt();
    QString proxy = settings.value(QStringLiteral("network/proxy")).toString();
    
    QStringList args;
    args << QStringLiteral("--timeout") << QStringLiteral("15") << QStringLiteral("--user-agent") << QStringLiteral(DEFAULT_UA);
    
    if (!proxy.isEmpty() && proxyType == NetworkAccessManager::HTTP_PROXY)
        args << QStringLiteral("--http-proxy") << proxy;
    else if (!proxy.isEmpty() && proxyType == NetworkAccessManager::SOCKS5_PROXY)
        args << QStringLiteral("--socks-proxy") << proxy;
    
    args << url.toString();
    m_process.start(userResourcesPath() + QStringLiteral("/ykdl-moonplayer"), args, QProcess::ReadOnly);
}


void ParserYkdl::parseOutput()
{
    QByteArray output = m_process.readAllStandardOutput();
#ifdef Q_OS_WIN
    output = QTextCodec::codecForLocale()->toUnicode(output).toUtf8();
#endif

    QJsonParseError json_error;
    QJsonDocument document = QJsonDocument::fromJson(output, &json_error);

    if (json_error.error != QJsonParseError::NoError)
    {
        showErrorDialog(QString::fromUtf8(m_process.readAllStandardError()));
        return;
    }

    // Select episode
    if (document.isArray())
    {
        QJsonArray episodes = document.array();
        QStringList titles;
        QList<QUrl> urls;

        for (const auto& item : episodes)
        {
            titles << item.toObject()[QStringLiteral("title")].toString();
            urls << item.toObject()[QStringLiteral("url")].toString();
        }

        Dialogs::instance()->selectionDialog(tr("Select episode"), titles, [=](int index) {
            Q_ASSERT(PlaylistModel::instance() != nullptr);
            PlaylistModel::instance()->addUrl(urls[index], m_download);
        });
        
        return;
    }

    // Video
    QJsonObject root = document.object();
    if (root.contains(QStringLiteral("streams")))
    {
        result.title = root[QStringLiteral("title")].toString();
        result.danmaku_url = root[QStringLiteral("danmaku_url")].toString();
        
        // get all available streams
        QJsonObject streams = root[QStringLiteral("streams")].toObject();
        for (auto i = streams.constBegin(); i != streams.constEnd(); i++)
        {
            QJsonObject item = i.value().toObject();
            
            // Basic stream infos
            Stream stream;
            stream.container = item[QStringLiteral("container")].toString();
            stream.referer = root[QStringLiteral("extra")].toObject()[QStringLiteral("referer")].toString();
            stream.ua = root[QStringLiteral("extra")].toObject()[QStringLiteral("ua")].toString();
            stream.is_dash = false;
            stream.seekable = true;
            
            // Write urls list
            QJsonArray urls = item[QStringLiteral("src")].toArray();
            if (urls.count() == 0)   // this stream is not available, skip it
            {
                continue;
            }

            for (const auto& url : urls)
            {
                stream.urls << QUrl(url.toString());
            }

            QString profile = item[QStringLiteral("video_profile")].toString();
            result.stream_types << QStringLiteral("%1 (%2)").arg(i.key(), profile);
            
            result.streams << stream;
        }
        finishParsing();
    }
    else
        showErrorDialog(QString::fromUtf8(m_process.readAllStandardError()));
}


