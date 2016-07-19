#include "playercore.h"
#include <QGraphicsVideoItem>
#include <QMediaPlayer>
#include <QMouseEvent>
#include <QNetworkRequest>
#include <QResizeEvent>
#include <QVideoWidget>

PlayerCore::PlayerCore(QWidget *parent) :
    QVideoWidget(parent), state(STOPPING), channel(CHANNEL_NORMAL)
{
    setFocusPolicy(Qt::StrongFocus);
    mediaPlayer = new QMediaPlayer(this);
    mediaPlayer->setVideoOutput(this);
    mediaPlayer->setNotifyInterval(5000);
    connect(mediaPlayer, &QMediaPlayer::stateChanged, this, &PlayerCore::onStateChanged);
    connect(mediaPlayer, &QMediaPlayer::positionChanged, this, &PlayerCore::onPositionChanged);
    connect(mediaPlayer, &QMediaPlayer::durationChanged, this, &PlayerCore::onDurationChanged);
    connect(mediaPlayer, SIGNAL(metaDataChanged(const QString&,const QVariant&)),
            this, SLOT(onMetaDataChanged(const QString&,const QVariant&)));
    switchingVideo = false;
    QList<QWidget*> widgets = findChildren<QWidget*>();
    foreach (QWidget *w, widgets) {
        w->setMouseTracking(true);
    }
}

PlayerCore::~PlayerCore()
{
}

void PlayerCore::mouseDoubleClickEvent(QMouseEvent *e)
{
    emit fullScreen();
    e->accept();
}

void PlayerCore::openFile(const QString &file, const QString &danmaku)
{
    if (state != STOPPING)
        switchingVideo = true; // Avoid emitting stopped() signal
    currentVideo = file;
    if (file.startsWith("http://") || file.startsWith("https://"))
    {
        QNetworkRequest request(file);
        request.setRawHeader("User-Agent", "moonplayer");
        mediaPlayer->setMedia(request);
    }
    else
        mediaPlayer->setMedia((QUrl::fromLocalFile(file)));
    mediaPlayer->play();
    state = VIDEO_PLAYING;
    currentPos = 0;
    emit played();
}

void PlayerCore::stop()
{
    if (state == STOPPING)
        return;
    mediaPlayer->stop();
}

void PlayerCore::changeState()
{
    if (state == VIDEO_PLAYING)
        mediaPlayer->pause();
    else if (state == VIDEO_PAUSING)
        mediaPlayer->play();
}

void PlayerCore::onStateChanged(QMediaPlayer::State mpState)
{
    switch (mpState) {
    case QMediaPlayer::StoppedState:
        state = STOPPING;
        if (switchingVideo)
            switchingVideo = false;
        else
            emit stopped();
        break;
    case QMediaPlayer::PlayingState:
        state = VIDEO_PLAYING;
        emit played();
        break;
    case QMediaPlayer::PausedState:
        state = VIDEO_PAUSING;
        emit paused();
        break;
    default:
        break;
    }
}

void PlayerCore::jumpTo(int pos)
{
    if (state == STOPPING)
        return;
    if (state == VIDEO_PLAYING)
        changeState();
    mediaPlayer->setPosition(((qint64)pos) * 1000);
}

void PlayerCore::setProgress(int pos)
{
    if (state == STOPPING)
        return;
    if (pos != currentPos)
    {
        currentPos = pos;
        qDebug("setProgress %i", pos);
        mediaPlayer->setPosition(((qint64)pos) * 1000);
    }
}

void PlayerCore::onPositionChanged(qint64 position)
{
    if (currentPos != position / 1000)
    {
        currentPos = position / 1000;
        emit timeChanged(currentPos);
    }
}

void PlayerCore::onDurationChanged(qint64 duration)
{
    emit lengthChanged(duration / 1000);
}

void PlayerCore::onMetaDataChanged(const QString &key, const QVariant &value)
{
    if (key == "Resolution")
        emit sizeChanged(value.toSize());
}

void PlayerCore::setVolume(int percentage)
{
    if (state == STOPPING)
        return;
    mediaPlayer->setVolume(percentage);
}

void PlayerCore::screenShot()
{
    qDebug("PlayerCore::screenShot: Unfinished function.");
}

void PlayerCore::speedUp()
{
    qDebug("PlayerCore::speedUp: Unfinished function.");
}

void PlayerCore::speedDown()
{
    qDebug("PlayerCore::speedDown: Unfinished function.");
}

void PlayerCore::speedSetToDefault()
{
    qDebug("PlayerCore::speedSetToDefault: Unfinished function.");
}
