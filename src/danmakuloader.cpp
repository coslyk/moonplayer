#include "danmakuloader.h"
#include "accessmanager.h"
#include "settings_danmaku.h"
#include "settings_player.h"
#include <QDir>
#include <QProcess>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTimer>
#include <QFile>
#include <QMessageBox>

DanmakuLoader::DanmakuLoader(QObject *parent) : QObject(parent)
{
    process = new QProcess(this);
    connect(process, static_cast<void (QProcess::*)(int)>(&QProcess::finished),
            this, &DanmakuLoader::onProcessFinished);
    reply = NULL;
}

void DanmakuLoader::reload()
{
    load(xmlFile, width, height);
}

void DanmakuLoader::load(const QString &xmlFile, int width, int height, double delay)
{
    this->xmlFile = xmlFile;
    this->width = width;
    this->height = height;
    this->delay = delay;
    if (reply) //another task is running
    {
        reply->abort();
        QTimer::singleShot(0, this, SLOT(reload())); //after event loop
        return;
    }
    QNetworkRequest request(xmlFile);
    request.setRawHeader("User-Agent", "moonplayer");
    reply = access_manager->get(request);
    connect(reply, &QNetworkReply::finished, this, &DanmakuLoader::onXmlDownloaded);
}

void DanmakuLoader::onXmlDownloaded()
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QStringList args;
        args << QDir(Settings::path).filePath("danmaku2ass") << "-o" << QDir::temp().filePath("moonplayer_danmaku.ass");
        args << "-s" << QString().sprintf("%dx%d", width, height);  //Ratio

        // Font
        if (!Settings::danmakuFont.isEmpty())
            args << "-fn" << Settings::danmakuFont;
#ifdef Q_OS_MAC
        else
            args << "-fn" << "PingFang SC";
#endif

        if (Settings::danmakuSize)
            args << "-fs" << QString::number(Settings::danmakuSize);
        else
        {
            if (width > 960)
                args << "-fs" << "36";
            else if (width > 640)
                args << "-fs" << "32";
            else
                args << "-fs" << "28";
        }

        // Duration of comment display
        if (Settings::durationScrolling)
            args << "-dm" << QString::number(Settings::durationScrolling);
        else
        {
            if (width > 960)
                args << "-dm" << "9";
            else if (width > 640)
                args << "-dm" << "7";
            else
                args << "-dm" << "6";
        }
        args << "-ds" << QString::number(Settings::durationStill);

        // Alpha
        args << "-a" << QString::number(Settings::danmakuAlpha);

        // Delay
        if (delay > 0.5)
            args << "--start-pos" << QString::number(delay);

        args << "/dev/stdin";
#ifdef Q_OS_MAC
        process->start("/usr/local/bin/python3", args);
#else
        process->start("python3", args);
#endif
        process->waitForStarted(-1);
        process->write(reply->readAll());
        process->closeWriteChannel();
    }
    reply->deleteLater();
    reply = NULL;
}

void DanmakuLoader::onProcessFinished()
{
    qDebug("%s", process->readAllStandardError().constData());
    emit finished(QDir::temp().filePath("moonplayer_danmaku.ass"));
}
