#include "yougetbridge.h"
#include "downloader.h"
#include "playlist.h"
#include "selectiondialog.h"
#include "settings_network.h"
#include "settings_plugins.h"
#include "videoqualities.h"
#include "webvideo.h"
#include <QApplication>
#include <QDir>
#include <QFile>
#include <QInputDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>
#include <QUrl>
#ifdef Q_OS_LINUX
#include "danmakudelaygetter.h"
#endif
#ifdef Q_OS_MAC
#include "settings_player.h"
#endif

YouGetBridge you_get_bridge;

YouGetBridge::YouGetBridge(QObject *parent) : QObject(parent)
{
    loadQualities();
    selectionDialog = NULL;
    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)),this, SLOT(onFinished()));
#ifdef Q_OS_MAC
    QStringList envs = QProcess::systemEnvironment();
    envs << "LC_CTYPE=en_US.UTF-8";
    process->setEnvironment(envs);
#endif
}

YouGetBridge::~YouGetBridge()
{
    saveQualities();
}


void YouGetBridge::parse(const QString &url, bool download, const QString &danmaku, const QString &format)
{
    if (selectionDialog == NULL)
        selectionDialog = new SelectionDialog;

    if (process->state() == QProcess::Running)
    {
        QMessageBox::warning(NULL, "Error", tr("Another file is being parsed."));
        return;
    }
    qDebug("Use you-get.");
    this->url = url;
    this->download = download;
    this->danmaku = danmaku;
    this->format = format;
    QStringList args;

#ifdef Q_OS_MAC
    QString sh_command = "you-get";
    if (!Settings::proxy.isEmpty())
        sh_command += QString(" --http-proxy '%1:%2'").arg(Settings::proxy,
                                                           QString::number(Settings::port));
    if (!format.isEmpty())
        sh_command += " --format=" + format;
    sh_command += " -t 10 --json " + url;
    args << "--login" << "-c" << sh_command;
    process->start("bash", args, QProcess::ReadOnly);
#else
    if (!Settings::proxy.isEmpty())
        args << "--http-proxy" << (Settings::proxy + ':' + QString::number(Settings::port));
    if (!format.isEmpty())
        args << "--format=" + format;
    args << "-t" << "10" << "--json" << url;
    process->start("you-get", args, QProcess::ReadOnly);
#endif
}


void YouGetBridge::onFinished()
{
    QJsonParseError json_error;
    QJsonObject obj = QJsonDocument::fromJson(process->readAllStandardOutput(), &json_error).object();
    if (json_error.error == QJsonParseError::NoError)
    {
        if (obj.contains("streams"))
        {
            QString title = obj["title"].toString();
            QJsonObject streams = obj["streams"].toObject();
            QJsonObject selectedItem;
            QJsonObject::const_iterator i;

            // Select video quality
            if (format.isEmpty()) // quality has not been selected
            {
                QString selected;
                QString host = QUrl(url).host();
                if (qualities.contains(host))
                    selected = qualities[host];
                else // Has not been saved
                {
                    QStringList items;
                    for (i = streams.constBegin(); i != streams.constEnd(); i++)
                    {
                        QString profile = i.value().toObject()["video_profile"].toString();
                        items << QString("%1 (%2)").arg(i.key(), profile);
                    }

                    selectionDialog->setList(items);
                    int state = selectionDialog->exec();
                    selected = selectionDialog->selectedItem();

                    if (state == QDialog::Rejected || selected.isEmpty())
                        return;
                    selected = selected.section(' ', 0, 0);

                    if (selectionDialog->remember()) // Save selection
                        qualities[QUrl(url).host()] = selected;
                }

                if (streams[selected].toObject().contains("src"))
                    selectedItem = streams[selected].toObject();
                else // get the video source of selected quality
                {
                    parse(url, download, danmaku, selected);
                    return;
                }
            }
            else // quality has been selected
            {
                for (i = streams.constBegin(); i != streams.constEnd(); i++)
                {
                    if (i.value().toObject().contains("src"))
                    {
                        selectedItem = i.value().toObject();
                        break;
                    }
                }
            }

            // Start playing or downloading
            if (!selectedItem.isEmpty())
            {
                QString container = selectedItem["container"].toString();
                QJsonArray json_urls = selectedItem["src"].toArray();
                QStringList names, urls;

                // Make file list
                for (int i = 0; i < json_urls.size(); i++)
                {
                    names << QString("%1_%2.%3").arg(title, QString::number(i), container);
                    urls << json_urls[i].toString();
                }

                // Download
                if (download)
                {
                     // Build file path list
                     QDir dir = QDir(Settings::downloadDir);
                     QString dirname = title + '.' + container;
                    if (urls.size() > 1)
                    {
                        if (!dir.cd(dirname))
                        {
                            dir.mkdir(dirname);
                            dir.cd(dirname);
                        }
                    }
                    for (int i = 0; i < names.size(); i++)
                         names[i] = dir.filePath(names[i]);

#ifdef Q_OS_LINUX
                    // Download more than 1 video clips with danmaku
                    if (!danmaku.isEmpty() && urls.size() > 1)
                        new DanmakuDelayGetter(names, urls, danmaku, true);
                    // Download without danmaku or only 1 clip with danmaku
                     else
                    {
                        for (int i = 0; i < urls.size(); i++)
                             downloader->addTask(urls[i].toUtf8(), names[i], urls.size() > 1, danmaku.toUtf8());
                    }
#else
                    for (int i = 0; i < urls.size(); i++)
                        downloader->addTask(urls[i].toUtf8(), names[i], urls.size() > 1);
#endif
                    QMessageBox::information(NULL, "Message", tr("Add download task successfully!"));
                }

                // Play
                else
                {
#ifdef Q_OS_LINUX
                    // Play more than 1 clips with danmaku
                    if (!danmaku.isEmpty() && urls.size() > 1)
                        new DanmakuDelayGetter(names, urls, danmaku, false);
                    // Play clips without danmaku or only 1 clip with danmaku
                    else
                    {
                        playlist->addFileAndPlay(names[0], urls[0], danmaku);
                        for (int i = 1; i < urls.size(); i++)
                            playlist->addFile(names[i], urls[i]);
                    }
#else
                    playlist->addFileAndPlay(names[0], urls[0]);
                    for (int i = 1; i < urls.size(); i++)
                         playlist->addFile(names[i], urls[i]);
#endif
                    if (Settings::autoCloseWindow)
                         webvideo->close();
                 }
                 return;
            }
        }
    }

    // Parse failed
#ifdef Q_OS_MAC
    if (QMessageBox::warning(NULL, "Error",
                         "Parse failed!\n" + QString::fromUtf8(process->readAllStandardError()),
                         tr("Cancel"),
                         tr("Upgrade Parser")))
    {
        QByteArray shFile = Settings::path.toUtf8() + "/upgrade-you-get.sh";
        if ((QFile::permissions(shFile) & QFile::ExeOther) == 0)
            system("chmod +x " + shFile);
        system("open -a Terminal.app " + shFile);
    }
#else
    QMessageBox::warning(NULL, "Error", "Parse failed!\n" + QString::fromUtf8(process->readAllStandardError()));
#endif
}
