#include "yougetbridge.h"
#include "downloader.h"
#include "playlist.h"
#include "settings_network.h"
#include "settings_plugins.h"
#include "webvideo.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QMessageBox>
#include <QProcess>

YouGetBridge you_get_bridge;

YouGetBridge::YouGetBridge(QObject *parent) : QObject(parent)
{
    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)),this, SLOT(onFinished()));
}

void YouGetBridge::parse(const QString &url, bool download)
{
    if (process->state() == QProcess::Running)
    {
        QMessageBox::warning(NULL, "Error", tr("Another file is being parsed."));
        return;
    }
    this->download = download;
    QStringList args;
    args << "--json" << url;
    process->start("you-get", args, QProcess::ReadOnly);
}

void YouGetBridge::onFinished()
{
    QJsonParseError json_error;
    QJsonDocument doc = QJsonDocument::fromJson(process->readAllStandardOutput(), &json_error);
    if (json_error.error == QJsonParseError::NoError)
    {
        if (doc.isObject())
        {
            QJsonObject obj = doc.object();
            if (obj.contains("streams"))
            {
                QString title = obj["title"].toString();
                QJsonObject streams = obj["streams"].toObject();
                QJsonObject::const_iterator i;
                for (i = streams.constBegin(); i != streams.constEnd(); i++)
                {
                    QJsonObject item = i.value().toObject();
                    if (item.contains("src"))
                    {
                        QString container = item["container"].toString();
                        QJsonArray urls = item["src"].toArray();
                        for (int i = 0; i < urls.size(); i++)
                        {
                            QString name = QString("%1_%2.%3").arg(title, QString::number(i), container);
                            if (download)
                            {
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
                                downloader->addTask(urls[i].toString().toUtf8(),
                                                    dir.filePath(name),
                                                    true);
                            }
                            else if (i == 0)
                                playlist->addFileAndPlay(name, urls[i].toString());
                            else
                                playlist->addFile(name, urls[i].toString());
                        }
                        if (download)
                            QMessageBox::information(NULL, "Message", tr("Add download task successfully!"));
                        else if (Settings::autoCloseWindow)
                            webvideo->close();
                        return;
                    }
                }
            }
        }
    }
    QMessageBox::warning(NULL, "Error", " Parse failed!\n" + QString::fromUtf8(process->readAllStandardError()));
}
