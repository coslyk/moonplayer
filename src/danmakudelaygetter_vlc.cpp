#include "danmakudelaygetter.h"
#include "downloader.h"
#include "playlist.h"
#include <QMessageBox>

libvlc_instance_t *DanmakuDelayGetter::vlcInstance = NULL;

DanmakuDelayGetter::DanmakuDelayGetter(QStringList &names, QStringList &urls, const QString &danmakuUrl,
                                       bool download, QObject *parent) :
    QObject(parent), names(names), urls(urls), danmakuUrl(danmakuUrl), download(download)
{
    vlcMedia = NULL;
    delay = 0;
    connect(this, &DanmakuDelayGetter::nextClip, this, &DanmakuDelayGetter::start, Qt::QueuedConnection);
    connect(this, &DanmakuDelayGetter::failed, this, &DanmakuDelayGetter::onFailed, Qt::QueuedConnection);
    start();
}

void DanmakuDelayGetter::start()
{
    QString name = names.takeFirst();
    QString url = urls.takeFirst();

    if (vlcMedia) // Prev clip is parsed
    {
        delay += libvlc_media_get_duration(vlcMedia) / 1000.0;
        libvlc_media_release(vlcMedia);
        vlcMedia = NULL;
    }

    // Add clip
    if (delay < 0.5)
    {
        if (download)
            downloader->addTask(url.toUtf8(), name, true, danmakuUrl.toUtf8());
        else
            playlist->addFileAndPlay(name, url, danmakuUrl);
    }
    else
    {
        if (download)
            downloader->addTask(url.toUtf8(), name, true,
                                QByteArray::number(delay) + ' ' + danmakuUrl.toUtf8());
        else
            playlist->addFile(name, url, QString::number(delay) + ' ' + danmakuUrl);
    }

    // Parse next clip
    if (!urls.isEmpty())
    {
        vlcMedia = libvlc_media_new_location(vlcInstance, url.toUtf8().constData());
        libvlc_event_manager_t *em = libvlc_media_event_manager(vlcMedia);
        libvlc_event_attach(em, libvlc_MediaParsedChanged, (libvlc_callback_t) onFinished, this);
        libvlc_media_parse_with_options(vlcMedia, libvlc_media_parse_network, 20000);
    }
    else // Finished
        deleteLater();
}

void DanmakuDelayGetter::onFinished(const libvlc_event_t *, DanmakuDelayGetter *d)
{
    switch (libvlc_media_get_parsed_status(d->vlcMedia)) {
    case libvlc_media_parsed_status_done:
        emit d->nextClip();
        break;
    case libvlc_media_parsed_status_timeout:
        qDebug("DanmakuDelayGetter: timeout.");
    case libvlc_media_parsed_status_failed:
        emit d->failed();
        break;
    default:
        break;
    }
}

void DanmakuDelayGetter::onFailed()
{
    QMessageBox::warning(NULL, "Error", tr("Failed to cut the danmaku!"));
    libvlc_media_release(vlcMedia);
    vlcMedia = NULL;
    while (!urls.isEmpty())
    {
        if (download)
            downloader->addTask(urls.takeFirst().toUtf8(), names.takeFirst(), true);
        else
            playlist->addFile(names.takeFirst(), urls.takeFirst());
    }
    deleteLater();
}
