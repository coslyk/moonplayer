#include "parserYkdl.h"
#include "accessManager.h"
#include <QDir>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTextCodec>
#include "platform/paths.h"

ParserYkdl ParserYkdl::s_instance;

ParserYkdl::ParserYkdl(QObject *parent) : ParserBase(parent)
{
    m_process = new QProcess(this);
    connect(m_process, SIGNAL(finished(int)),this, SLOT(parseOutput()));
    connect(m_process, &QProcess::errorOccurred, [=](){ showErrorDialog(m_process->errorString()); });
}

ParserYkdl::~ParserYkdl()
{
    if (m_process->state() == QProcess::Running)
    {
        m_process->kill();
        m_process->waitForFinished();
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
    if (m_process->state() == QProcess::Running)
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
    m_process->start(userResourcesPath() + QStringLiteral("/ykdl-moonplayer"), args, QProcess::ReadOnly);
}


void ParserYkdl::parseOutput()
{
    QByteArray output = m_process->readAllStandardOutput();
#ifdef Q_OS_WIN
    output = QTextCodec::codecForLocale()->toUnicode(output).toUtf8();
#endif

    QJsonParseError json_error;
    QJsonDocument document = QJsonDocument::fromJson(output, &json_error);

    if (json_error.error != QJsonParseError::NoError)
    {
        showErrorDialog(QString::fromUtf8(m_process->readAllStandardError()));
        return;
    }

    // Select episode
    if (document.isArray())
    {
        QVariantList episodes = document.toVariant().toList();
        QStringList titles;
        QList<QUrl> urls;
        foreach (QVariant item, episodes)
        {
            titles << item.toHash()[QStringLiteral("title")].toString();
            urls << item.toHash()[QStringLiteral("url")].toString();
        }
        selectEpisode(titles, urls);
        return;
    }

    // Video
    QVariantHash obj = document.toVariant().toHash();
    if (obj.contains(QStringLiteral("streams")))
    {
        result.title = obj[QStringLiteral("title")].toString();
        result.danmaku_url = obj[QStringLiteral("danmaku_url")].toString();
        
        // get all available streams
        QVariantHash streams = obj[QStringLiteral("streams")].toHash();
        for (auto i = streams.constBegin(); i != streams.constEnd(); i++)
        {
            QString profile = i.value().toHash()[QStringLiteral("video_profile")].toString();
            result.stream_types << QStringLiteral("%1 (%2)").arg(i.key(), profile);
            
            // Basic stream infos
            QVariantHash item = i.value().toHash();
            Stream stream;
            stream.container = item[QStringLiteral("container")].toString();
            stream.referer = obj[QStringLiteral("extra")].toHash()[QStringLiteral("referer")].toString();
            stream.ua = obj[QStringLiteral("extra")].toHash()[QStringLiteral("ua")].toString();
            stream.is_dash = false;
            stream.seekable = true;
            
            // Write urls list
            QVariantList urls = item[QStringLiteral("src")].toList();
            if (urls.count() == 0)   // this stream is not available, skip it
                continue;
            for (int i = 0; i < urls.size(); i++)
                stream.urls << urls[i].toUrl();
            
            result.streams << stream;
        }
        finishParsing();
    }
    else
        showErrorDialog(QString::fromUtf8(m_process->readAllStandardError()));
}


