#include "ykdlbridge.h"
#include "accessmanager.h"
#include "downloader.h"
#include "platforms.h"
#include "playlist.h"
#include "selectiondialog.h"
#include "settings_network.h"
#include "reslibrary.h"
#include <QApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>
#include <QUrl>
#include "terminal.h"

YkdlBridge ykdl_bridge;

YkdlBridge::YkdlBridge(QObject *parent) : QObject(parent)
{
    selectionDialog = NULL;
    process = new QProcess(this);

    QStringList envs = QProcess::systemEnvironment();
    envs << "PYTHONIOENCODING=utf8";

#if defined(Q_OS_MAC)
    envs << QString("PYTHONPATH=%1/Library/Application Support/MoonPlayer/ykdl/").arg(QDir::homePath());
    envs << "LC_CTYPE=en_US.UTF-8";
#elif defined(Q_OS_LINUX)
    envs << QString("PYTHONPATH=%1/.moonplayer/ykdl/").arg(QDir::homePath());
#else
#error ERROR: Unsupported system!
#endif
    process->setEnvironment(envs);
    connect(process, SIGNAL(finished(int)),this, SLOT(onFinished()));
    connect(process, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onError()));
}

YkdlBridge::~YkdlBridge()
{
    if (process->state() == QProcess::Running)
    {
        process->kill();
        process->waitForFinished();
    }
}


void YkdlBridge::parse(const QString &url, bool download)
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

    QStringList args;
    args << "-m" << "cykdl" << "-t" << "10" << "--json";
    if (Settings::proxyType == "http" && !Settings::proxy.isEmpty())
        args << "--proxy" << (Settings::proxy + ':' + QString::number(Settings::port));
    args << url;
    process->start("python", args, QProcess::ReadOnly);
}


void YkdlBridge::onFinished()
{
    QJsonParseError json_error;
    QByteArray output = process->readAllStandardOutput();

    QJsonObject obj = QJsonDocument::fromJson(output, &json_error).object();
    if (json_error.error == QJsonParseError::NoError)
    {
        if (obj.contains("streams"))
        {
            QString title = obj["title"].toString();
            QJsonObject streams = obj["streams"].toObject();
            QJsonObject selectedItem;
            QJsonObject::const_iterator i;

            // Select video quality
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
            selectedItem = streams[selected].toObject();

            // Start playing or downloading
            if (!selectedItem.isEmpty())
            {
                QJsonArray json_urls = selectedItem["src"].toArray();
                QStringList names, urls;
                QString container = selectedItem["container"].toString();
                QString referer = obj["extra"].toObject()["referer"].toString();
                QString ua = obj["extra"].toObject()["ua"].toString();

                if (json_urls.size() == 0)
                {
                    QMessageBox::warning(NULL,
                                         "Error",
                                         tr("No videos available. Please try other language or quality."));
                    return;
                }

                // replace illegal chars in title with .
                static QRegularExpression illegalChars("[\\\\/]");
                title.replace(illegalChars, ".");


                // Bind referer and use-agent
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
                         names[i] = dir.filePath(QString(names[i]));

                    for (int i = 0; i < urls.size(); i++)
                         downloader->addTask(urls[i].toUtf8(), names[i], urls.size() > 1);

                    QMessageBox::information(NULL, "Message", tr("Add download task successfully!"));
                }

                // Play
                else
                {
                    playlist->addFileAndPlay(names[0], urls[0]);
                    for (int i = 1; i < urls.size(); i++)
                        playlist->addFile(names[i], urls[i]);
                    res_library->close();
                }
                return;
            }
        }
    }

    // Parse failed
    onError();
}

void YkdlBridge::onError()
{
    if (QMessageBox::warning(NULL, "Error",
                             "Parse failed!\nURL:" + url + "\n" +
                             QString::fromUtf8(process->readAllStandardError()),
                             tr("Cancel"),
                             tr("Upgrade Parser")))
        updateYkdl();
}


void YkdlBridge::updateYkdl()
{
    execShell(ykdlUpgraderPath());
}

