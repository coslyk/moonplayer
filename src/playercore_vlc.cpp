#include "playercore.h"
#include <QCoreApplication>
#include <QFileDialog>
#include <QMouseEvent>
#include <QTimer>

PlayerCore *player_core = NULL;

PlayerCore::PlayerCore(QWidget *parent) :
    QWidget(parent), state(STOPPING), channel(CHANNEL_NORMAL)
{
    const char *vlc_args[] = {
        "--http-user-agent=moonplayer"
    };
    setenv("VLC_PLUGIN_PATH", (QCoreApplication::applicationDirPath() + "/plugins").toUtf8().constData(), 1);

    // Set color and focus policy
    setPalette(QPalette(QColor(0, 0, 0)));
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);

    // Set timer to update progress
    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &PlayerCore::updateProgress);

    // Fix libvlc's bug:
    // It freezes when calling a libvlc's API function in a libvlc-callback function
    connect(this, &PlayerCore::playNext, this, &PlayerCore::openFile, Qt::QueuedConnection);
    connect(this, &PlayerCore::stopNeeded, this, &PlayerCore::stop, Qt::QueuedConnection);
    next_waited = false;

    // Create VLC Player instance
    vlcInstance = libvlc_new(sizeof(vlc_args) / sizeof(char*), vlc_args);
    vlcPlayer = libvlc_media_player_new(vlcInstance);
    libvlc_media_player_set_nsobject(vlcPlayer, (void*) winId());
    vlcMedia = NULL;

    eventManager = libvlc_media_player_event_manager(vlcPlayer);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerStopped, (libvlc_callback_t) onStopped, this);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerPaused, (libvlc_callback_t) onPaused, this);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerPlaying, (libvlc_callback_t) onPlayed, this);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerLengthChanged, (libvlc_callback_t) onLengthChanged, this);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerEndReached, (libvlc_callback_t) onEndReached, this);
    libvlc_event_attach(eventManager, libvlc_MediaPlayerVout, (libvlc_callback_t) onSizeChanged, this);

    player_core = this;
}

void PlayerCore::closeEvent(QCloseEvent *event)
{
    if (state != STOPPING)
    {
        timer->stop();
        libvlc_event_detach(eventManager, libvlc_MediaPlayerStopped, (libvlc_callback_t) onStopped, this);
        libvlc_media_player_stop(vlcPlayer);
    }
    event->accept();
}

PlayerCore::~PlayerCore()
{
    libvlc_media_player_release(vlcPlayer);
    vlcPlayer = NULL;
}

void PlayerCore::mouseDoubleClickEvent(QMouseEvent *e)
{
    emit fullScreen();
    e->accept();
}

void PlayerCore::openFile(const QString &file, const QString &danmaku)
{
    this->file = file;
    this->danmaku = danmaku;
    if (state != STOPPING)
    {
        next_waited = true;
        stop();
        return;
    }
    if (file.contains("://"))
        vlcMedia = libvlc_media_new_location(vlcInstance, file.toUtf8().constData());
    else
        vlcMedia = libvlc_media_new_path(vlcInstance, file.toUtf8().constData());
    libvlc_media_player_set_media(vlcPlayer, vlcMedia);
    libvlc_media_player_play(vlcPlayer);
    timer->start(1000);
}

void PlayerCore::onLengthChanged(const libvlc_event_t *, PlayerCore *c)
{
    int length = libvlc_media_player_get_length(c->vlcPlayer) / 1000;
    emit c->lengthChanged(length);
}

void PlayerCore::onSizeChanged(const libvlc_event_t *, PlayerCore *c)
{
    int width = libvlc_video_get_width(c->vlcPlayer);
    int height = libvlc_video_get_height(c->vlcPlayer);
    emit c->sizeChanged(QSize(width, height));
}

void PlayerCore::stop()
{
    if (state == STOPPING)
        return;
    libvlc_media_player_stop(vlcPlayer);
}

void PlayerCore::onStopped(const libvlc_event_t *, PlayerCore *c)
{
    c->state = STOPPING;
    libvlc_media_release(c->vlcMedia);
    c->vlcMedia = NULL;
    c->timer->stop();
    if (c->next_waited)
    {
        c->next_waited = false;
        emit c->playNext(c->file, c->danmaku);
    }
    else
        emit c->stopped();
}

void PlayerCore::onEndReached(const libvlc_event_t *, PlayerCore *c)
{
    // Cannot call libvlc_media_player_stop() directly, or it may cause freeze.
    // It's a libvlc's bug.
    emit c->stopNeeded();
}

void PlayerCore::changeState()
{
    if (state == VIDEO_PLAYING)
    {
        libvlc_media_player_set_pause(vlcPlayer, 1);
        timer->stop();
    }
    else if (state == VIDEO_PAUSING)
    {
        libvlc_media_player_set_pause(vlcPlayer, 0);
        timer->start(1000);
    }
}

void PlayerCore::onPlayed(const libvlc_event_t *, PlayerCore *c)
{
    c->state = VIDEO_PLAYING;
    emit c->played();
}

void PlayerCore::onPaused(const libvlc_event_t *, PlayerCore *c)
{
    c->state = VIDEO_PAUSING;
    emit c->paused();
}

void PlayerCore::setVolume(int volume)
{
    libvlc_audio_set_volume(vlcPlayer, volume * 10);
}

void PlayerCore::setProgress(int pos)
{
    if (state == STOPPING)
        return;
    int currentPos = libvlc_media_player_get_time(vlcPlayer) / 1000;
    if (currentPos != pos)
        libvlc_media_player_set_time(vlcPlayer, pos * 1000);
}

void PlayerCore::updateProgress()
{
    if (state == STOPPING)
        return;
    int pos = libvlc_media_player_get_time(vlcPlayer) / 1000;
    emit timeChanged(pos);
}

void PlayerCore::jumpTo(int pos)
{
    if (state == STOPPING)
        return;
    if (state == VIDEO_PLAYING)
        changeState();
    libvlc_media_player_set_position(vlcPlayer, pos);
}

void PlayerCore::screenShot()
{
    if (state == STOPPING)
        return;
    if (state == VIDEO_PLAYING)
        changeState();
    QString filename = QFileDialog::getSaveFileName(this, "Save snapshot", QDir::homePath());
    if (!filename.isEmpty())
        libvlc_video_take_snapshot(vlcPlayer, 0, filename.toUtf8().constData(), 0, 0);
}

void PlayerCore::speedDown()
{
    if (state == STOPPING)
        return;
    qDebug("PlayerCore::speedDown");
}

void PlayerCore::speedUp()
{
    if (state == STOPPING)
        return;
    qDebug("PlayerCore::speedUp");
}

void PlayerCore::speedSetToDefault()
{
    if (state == STOPPING)
        return;
    qDebug("PlayerCore::speedSetToDefault");
}
