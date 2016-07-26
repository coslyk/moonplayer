#include "danmakudelaygetter.h"
#include "downloader.h"
#include "playlist.h"
#include <QProcess>

bool DanmakuDelayGetter::dummy_mode = true;

DanmakuDelayGetter::DanmakuDelayGetter(QStringList &names, QStringList &urls,
                                       const QString &danmakuUrl, bool download, QObject *parent) :
    QObject(parent), names(names), urls(urls), danmakuUrl(danmakuUrl), download(download)
{
    process = new QProcess(this);
    connect(process, SIGNAL(finished(int)), this, SLOT(onFinished()));
    delay = 0;
    start();
}

void DanmakuDelayGetter::start()
{
    QStringList args;
    if (dummy_mode)
        args << "-nosound" << "-vc" << "dummy" << "-vo" << "null" << "-identify" << urls.first();
    else
        args << "-nosound" << "-vc" << "black" << "-vo" << "null" << "-endpos" << "0:01" << "-identify" << urls.first();
    process->start("mplayer", args);
}

void DanmakuDelayGetter::onFinished()
{
    QString output = QString::fromUtf8(process->readAllStandardOutput());

    // If player_core has no "dummy" video codec, use "black" instead
    if (dummy_mode && !output.contains("ID_LENGTH="))
    {
        dummy_mode = false;
        start();
        return;
    }

    double length = output.section("ID_LENGTH=", 1).section('\n', 0, 0).toDouble();
    if (delay < 0.5) // first video clip
    {
        if (download)
            downloader->addTask(urls.takeFirst().toUtf8(), names.takeFirst(), true, danmakuUrl.toUtf8());
        else
            playlist->addFileAndPlay(names.takeFirst(), urls.takeFirst(), danmakuUrl);
    }
    else
    {
        if (download)
            downloader->addTask(urls.takeFirst().toUtf8(), names.takeFirst(), true,
                                QByteArray::number(delay) + ' ' + danmakuUrl.toUtf8());
        else
            playlist->addFile(names.takeFirst(), urls.takeFirst(), QString::number(delay) + ' ' + danmakuUrl);
    }

    delay += length;
    if (names.count() == 1) // the last clip
    {
        if (download)
            downloader->addTask(urls.takeFirst().toUtf8(), names.takeFirst(), true,
                                QByteArray::number(delay) + ' ' + danmakuUrl.toUtf8());
        else
            playlist->addFile(names.takeFirst(), urls.takeFirst(), QString::number(delay) + ' ' + danmakuUrl);
    }

    if (names.isEmpty())
        deleteLater();
    else
        start();
}
