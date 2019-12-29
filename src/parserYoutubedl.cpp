#include "parserYoutubedl.h"
#include "accessManager.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
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
    QJsonObject obj = QJsonDocument::fromJson(output, &json_error).object();
    if (json_error.error != QJsonParseError::NoError)
    {
        showErrorDialog(QString::fromUtf8(process->readAllStandardError()));
        return;
    }
    if (obj.contains("formats"))
    {
        result.title = obj["title"].toString();
        QJsonArray formats = obj["formats"].toArray();
        QHash<QString, QJsonObject> formatsHash;
        QStringList formatsList;

        // Select video quality
        // get all available qualities
        QString bestMp4Audio, bestWebmAudio;
        int bestMp4AudioSize = 0;
        int bestWebmAudioSize = 0;
        for (int i = 0; i < formats.size(); i++)
        {
            QJsonObject item = formats[i].toObject();
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
                formatsList << formatName;
                formatsHash[formatName] = item;
            }
        }

        // show dialog
        int index = selectQuality(formatsList);
        if (index == -1)
            return;
        QString selected = formatsList[index];
        QJsonObject selectedItem = formatsHash[selected];

        // write info
        result.container = selectedItem["protocol"].toString() == "m3u8" ? "m3u8" : selectedItem["ext"].toString();
        result.referer = selectedItem["http_headers"].toObject()["Referer"].toString();
        QString ua = selectedItem["http_headers"].toObject()["User-Agent"].toString();
        
        if (ua != DEFAULT_UA)
            result.ua = ua;
        
        result.urls << selectedItem["url"].toString();

        // Video has no audio track?
        if (selectedItem["acodec"] == "none")
        {
            if (result.container == "webm" && !bestWebmAudio.isEmpty())
            {
                result.is_dash = true;
                result.urls << bestWebmAudio;
            }
            else if (result.container == "mp4" && !bestMp4Audio.isEmpty())
            {
                result.is_dash = true;
                result.urls << bestMp4Audio;
            }
            else
            {
                showErrorDialog(tr("The video of selected quality has no audio track. Please select another one."));
                return;
            }
        }
        finishParsing();
    }
    else
        showErrorDialog(QString::fromUtf8(process->readAllStandardError()));
}
