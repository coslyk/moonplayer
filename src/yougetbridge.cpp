#include "yougetbridge.h"
#include "accessmanager.h"
#include "downloader.h"
#include "platforms.h"
#include "playlist.h"
#include "selectiondialog.h"
#include "settings_network.h"
#include "settings_plugins.h"
#include "reslibrary.h"
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
#include <QTextCodec>
#include <QUrl>
#include "danmakudelaygetter.h"
#include "terminal.h"

YouGetBridge you_get_bridge;

YouGetBridge::YouGetBridge(QObject *parent) : QObject(parent)
{
    selectionDialog = NULL;
    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)),this, SLOT(onFinished()));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onError()));

#ifdef Q_OS_MAC
    // add "/usr/local/bin" to path
    QStringList envs = QProcess::systemEnvironment();
    for (int i = 0; i < envs.size(); i++)
    {
        if (envs[i].startsWith("PATH=") && !envs[i].contains("/usr/local/bin"))
        {
            envs[i] += ":/usr/local/bin";
            break;
        }
    }
    envs << "LC_CTYPE=en_US.UTF-8";
    process->setEnvironment(envs);
#endif
}

YouGetBridge::~YouGetBridge()
{
    if (process->state() == QProcess::Running)
    {
        process->kill();
        process->waitForFinished();
    }
}


void YouGetBridge::parse(const QString &url, bool download, const QString &format)
{
    if (selectionDialog == NULL)
        selectionDialog = new SelectionDialog;

    if (process->state() == QProcess::Running)
    {
        QMessageBox::warning(NULL, "Error", tr("Another file is being parsed."));
        return;
    }
    this->url = url;
    this->download = download;
    this->format = format;

    QStringList args;
    args << "python3" << yougetFilePath();
    if (Settings::proxyType != "no" && !Settings::proxy.isEmpty())
    {
        args << (Settings::proxyType == "socks5" ? "--socks-proxy" : "--http-proxy");
        args << (Settings::proxy + ':' + QString::number(Settings::port));
    }
    if (!format.isEmpty())
        args << "--format=" + format;
    args << "-t" << "10" << "--json" << url;
    process->start("/usr/bin/env", args, QProcess::ReadOnly);
}


void YouGetBridge::onFinished()
{
    QJsonParseError json_error;
    QByteArray output = process->readAllStandardOutput();
#ifdef  Q_OS_WIN
    QTextCodec *codec = QTextCodec::codecForLocale();
    output = codec->toUnicode(output).toUtf8();
#endif //  Q_OS_WIN

    QJsonObject obj = QJsonDocument::fromJson(output, &json_error).object();
    if (json_error.error == QJsonParseError::NoError)
    {
        if (obj.contains("audiolang")) // select languages
        {
            QJsonArray langs = obj["audiolang"].toArray();
            QStringList langs_list;
            QHash<QString,QString> lang2url;
            QString selected;
            for (int i = 0; i < langs.size(); i++)
            {
                QString name = langs[i].toObject()["lang"].toString();
                langs_list << name;
                lang2url[name] = langs[i].toObject()["url"].toString();
            }
            if (langs_list.size() > 1)
                selected = selectionDialog->showDialog(langs_list, tr("Please select a language:"));
            if (!selected.isEmpty())
            {
                QString new_url = lang2url[selected];
                if (!url.startsWith(new_url))
                {
                    parse(new_url, download);
                    return;
                }
            }
        }

        if (obj.contains("streams"))
        {
            QString title = obj["title"].toString();
            QJsonObject streams = obj["streams"].toObject();
            QJsonObject selectedItem;
            QJsonObject::const_iterator i;

            // Select video quality
            if (format.isEmpty()) // quality has not been selected
            {
                // get all available qualities
                QString selected;
                QStringList items;
                for (i = streams.constBegin(); i != streams.constEnd(); i++)
                {
                    QString profile = i.value().toObject()["video_profile"].toString();
                    items << QString("%1 (%2)").arg(i.key(), profile);
                }

                // show dialog
                selected = selectionDialog->showDialog(items,
                                                       tr("Please select a video quality:"));
                if (selected.isEmpty())
                    return;
                selected = selected.section(' ', 0, 0);

                // Check if re-parsing is needed
                if (streams[selected].toObject().contains("src"))
                    selectedItem = streams[selected].toObject();
                else
                {
                    parse(url, download, selected);
                    return;
                }
            }
            else // quality has been selected
                selectedItem = streams[format].toObject();

            // Start playing or downloading
            if (!selectedItem.isEmpty())
            {
                QString container = selectedItem["container"].toString();
                QJsonArray json_urls = selectedItem["src"].toArray();
                QStringList names, urls;
                QString referer, ua, danmaku_url;
                bool seekable = obj["seekable"].toBool();
                if (obj.contains("referer"))
                    referer = obj["referer"].toString();
                if (obj.contains("user-agent"))
                    ua = obj["user-agent"].toString();
                if (obj.contains("danmaku_url"))
                    danmaku_url = obj["danmaku_url"].toString();
                qDebug("Referer: %s\nUA: %s\nDanmaku: %s",
                       referer.toUtf8().constData(),
                       ua.toUtf8().constData(),
                       danmaku_url.toUtf8().constData());

                if (json_urls.size() == 0)
                {
                    QMessageBox::warning(NULL,
                                         "Error",
                                         tr("No videos available. Please try other language or quality."));
                    return;
                }

                // Make file list
                for (int i = 0; i < json_urls.size(); i++)
                {
                    names << QString("%1_%2.%3").arg(title, QString::number(i), container);
                    urls << json_urls[i].toString();
                }

                // Bind referer and use-agent, set unseekable-list
                if (!referer.isEmpty())
                {
                    foreach (QString url, urls)
                        referer_table[QUrl(url).host()] = referer.toUtf8();
                }
                if (!ua.isEmpty())
                {
                    foreach (QString url, urls)
                        ua_table[QUrl(url).host()] = ua.toUtf8();
                }
                if (!seekable)
                {
                    foreach (QString url, urls) {
                        unseekable_hosts.append(QUrl(url).host());
                    }
                }

                // Download
                if (download)
                {
                    static QRegularExpression illegalChars("[\\\\/]");
                    title.replace(illegalChars, ".");
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
                         names[i] = dir.filePath(QString(names[i]).replace(illegalChars, "."));

                    // Download more than 1 video clips with danmaku
                    if (!danmaku_url.isEmpty() && urls.size() > 1)
                        new DanmakuDelayGetter(names, urls, danmaku_url, true);
                    // Download without danmaku or only 1 clip with danmaku
                    else
                    {
                        for (int i = 0; i < urls.size(); i++)
                             downloader->addTask(urls[i].toUtf8(), names[i], urls.size() > 1, danmaku_url.toUtf8());
                    }
                    QMessageBox::information(NULL, "Message", tr("Add download task successfully!"));
                }

                // Play
                else
                {
                    // Play more than 1 clips with danmaku
                    if (!danmaku_url.isEmpty() && urls.size() > 1)
                        new DanmakuDelayGetter(names, urls, danmaku_url, false);
                    // Play clips without danmaku or only 1 clip with danmaku
                    else
                    {
                        playlist->addFileAndPlay(names[0], urls[0], danmaku_url);
                        for (int i = 1; i < urls.size(); i++)
                            playlist->addFile(names[i], urls[i]);
                    }

                    res_library->close();
                 }
                 return;
            }
        }
    }

    // Parse failed
    onError();
}

void YouGetBridge::onError()
{
    if (QMessageBox::warning(NULL, "Error",
                             "Parse failed!\nURL:" + url + "\n" +
                             QString::fromUtf8(process->readAllStandardError()),
                             tr("Cancel"),
                             tr("Upgrade Parser")))
        updateYouGet();
}


void YouGetBridge::updateYouGet()
{
    execShell(yougetUpgraderPath());
}

