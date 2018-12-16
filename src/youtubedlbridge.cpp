#include "youtubedlbridge.h"
#include "accessmanager.h"
#include "platforms.h"
#include "selectiondialog.h"
#include "settings_network.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>
#include <QRegularExpression>

YoutubeDLBridge youtubedl_bridge;

YoutubeDLBridge::YoutubeDLBridge(QObject *parent) : ParserBridge(parent)
{
    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)),this, SLOT(parseOutput()));
}

YoutubeDLBridge::~YoutubeDLBridge()
{
    if (process->state() == QProcess::Running)
    {
        process->kill();
        process->waitForFinished();
    }
}


void YoutubeDLBridge::runParser(const QString &url)
{
    if (process->state() == QProcess::Running)
    {
        QMessageBox::warning(NULL, "Error", tr("Another file is being parsed."));
        return;
    }

    QStringList args;
    args << "-j" << "--user-agent" << DEFAULT_UA;
    if (!Settings::proxy.isEmpty() &&
            (Settings::proxyType == "http" ||
             (Settings::proxyType == "http_unblockcn" && !url.contains(".youtube.com"))))
        args << "--proxy" << (Settings::proxy + ':' + QString::number(Settings::port));
    else if (Settings::proxyType == "socks5")
        args << "--proxy" << QString("socks5://%1:%2/").arg(Settings::proxy, QString::number(Settings::port));
    args << url;
    process->start("youtube-dl", args, QProcess::ReadOnly);
}


void YoutubeDLBridge::parseOutput()
{
    QByteArray output = process->readAllStandardOutput();
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
        QString selected = selectionDialog->showDialog(formatsList,
                                                   tr("Please select a video quality:"));
        if (selected.isEmpty())
            return;
        QJsonObject selectedItem = formatsHash[selected];

        // write info
        result.container = selectedItem["ext"].toString();
        result.referer = selectedItem["http_headers"].toObject()["Referer"].toString();
        QString ua = selectedItem["http_headers"].toObject()["User-Agent"].toString();
        if (ua != DEFAULT_UA)
        {
            qDebug("Use other UA: %s", ua.toUtf8().constData());
            result.ua = ua;
        }
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
