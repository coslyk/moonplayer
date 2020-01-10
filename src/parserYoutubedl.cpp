#include "parserYoutubedl.h"
#include "accessManager.h"
#include <QDir>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>
#include <QSettings>
#include <QTextCodec>
#include "platform/paths.h"

ParserYoutubeDL ParserYoutubeDL::s_instance;

ParserYoutubeDL::ParserYoutubeDL(QObject *parent) : ParserBase(parent)
{
    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)),this, SLOT(parseOutput()));
    connect(process, &QProcess::errorOccurred, [=](){ showErrorDialog(process->errorString()); });
}

ParserYoutubeDL::~ParserYoutubeDL()
{
    if (process->state() == QProcess::Running)
    {
        process->kill();
        process->waitForFinished();
    }
}


void ParserYoutubeDL::runParser(const QUrl& url)
{
    if (process->state() == QProcess::Running)
    {
        QMessageBox::warning(nullptr, "Error", tr("Another file is being parsed."));
        return;
    }
    
    QSettings settings;
    NetworkAccessManager::ProxyType proxyType = (NetworkAccessManager::ProxyType) settings.value("network/proxy_type").toInt();
    QString proxy = settings.value("network/proxy").toString();

    QStringList args;
    args << "-j" << "--user-agent" << DEFAULT_UA;
    if (!proxy.isEmpty() && proxyType == NetworkAccessManager::HTTP_PROXY)
        args << "--proxy" << proxy;
    else if (!proxy.isEmpty() && proxyType == NetworkAccessManager::SOCKS5_PROXY)
        args << "--proxy" << QString("socks5://%1/").arg(proxy);

    args << url.toString();
    process->start(userResourcesPath() + "/youtube-dl", args, QProcess::ReadOnly);
}


void ParserYoutubeDL::parseOutput()
{
    QByteArray output = process->readAllStandardOutput();
#ifdef Q_OS_WIN
    output = QTextCodec::codecForLocale()->toUnicode(output).toUtf8();
#endif

    QJsonParseError json_error;
    QVariantHash obj = QJsonDocument::fromJson(output, &json_error).toVariant().toHash();
    if (json_error.error != QJsonParseError::NoError)
    {
        showErrorDialog(QString::fromUtf8(process->readAllStandardError()));
        return;
    }
    
    if (obj.contains("formats"))
    {
        result.title = obj["title"].toString();
        
        // Get all available streams
        QVariantList formats = obj["formats"].toList();
        QVariantList streams;
        QString bestMp4Audio, bestWebmAudio;
        int bestMp4AudioSize = 0;
        int bestWebmAudioSize = 0;
        for (int i = 0; i < formats.size(); i++)
        {
            QVariantHash item = formats[i].toHash();
            
            // DASH Audio
            if (item["vcodec"].toString() == "none")
            {
                if (item["ext"].toString() == "webm" && item["filesize"].toInt() > bestWebmAudioSize)
                {
                    bestWebmAudio = item["url"].toString();
                    bestWebmAudioSize = item["filesize"].toInt();
                }
                else if (item["ext"].toString() == "m4a" && item["filesize"].toInt() > bestMp4AudioSize)
                {
                    bestMp4Audio = item["url"].toString();
                    bestMp4AudioSize = item["filesize"].toInt();
                }
            }
            
            // Videos
            else
            {
                QString formatName = QString("%1 (%2)").arg(item["format"].toString(), item["ext"].toString());
                result.stream_types << formatName;
                streams << item;
            }
        }

        // Fill stream infos
        foreach (QVariant i, streams)
        {
            QVariantHash item = i.toHash();
            Stream stream;
            
            // Basic stream infos
            stream.container = item["protocol"].toString() == "m3u8" ? "m3u8" : item["ext"].toString();
            stream.referer = item["http_headers"].toHash()["Referer"].toString();
            stream.seekable = true;
            stream.is_dash = false;
            
            QString ua = item["http_headers"].toHash()["User-Agent"].toString();
            if (ua != DEFAULT_UA)
                stream.ua = ua;
            
            // Urls
            stream.urls << item["url"].toString();

            // Video has no audio track? => Dash video, audio in seperate file
            if (item["acodec"] == "none")
            {
                if (stream.container == "webm" && !bestWebmAudio.isEmpty())
                {
                    stream.is_dash = true;
                    stream.urls << bestWebmAudio;
                }
                else if (stream.container == "mp4" && !bestMp4Audio.isEmpty())
                {
                    stream.is_dash = true;
                    stream.urls << bestMp4Audio;
                }
                else
                {
                    continue;
                }
            }
            
            result.streams << stream;
        }
        
        finishParsing();
    }
    else
        showErrorDialog(QString::fromUtf8(process->readAllStandardError()));
}
