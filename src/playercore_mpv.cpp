#include "playercore.h"
#include "danmakuloader.h"
#include "settings_audio.h"
#include "settings_network.h"
#include "settings_player.h"
#include "settings_video.h"
#include "accessmanager.h"
#include <stdio.h>
#include <mpv/client.h>
#include <QCoreApplication>
#include <QDir>
#include <QEvent>
#include <QHash>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>

static void postEvent(void *ptr)
{
    PlayerCore *core = (PlayerCore*) ptr;
    QCoreApplication::postEvent(core, new QEvent(QEvent::User));
}

PlayerCore *player_core = NULL;

static QHash<QString,int64_t> unfinished_time;

PlayerCore::PlayerCore(QWidget *parent) :
    QWidget(parent)
{
    printf("Initialize mpv backend...\n");

    // Set color and focus policy
    setPalette(QPalette(QColor(0, 0, 0)));
    setAutoFillBackground(true);
    setFocusPolicy(Qt::StrongFocus);

    // create mpv instance
    mpv = mpv_create();
    if (!mpv)
    {
        qDebug("Cannot create mpv instance.");
        exit(-1);
    }

    // set mpv options
    int64_t wid = winId();
    mpv_set_option(mpv, "wid", MPV_FORMAT_INT64, &wid);   // playback's window id
    mpv_set_option_string(mpv, "softvol", "yes");         // mpv handles the volume
    mpv_set_option_string(mpv, "input-cursor", "no");     // We handle cursor
    mpv_set_option_string(mpv, "cursor-autohide", "no");
    mpv_set_option_string(mpv, "osc", "no");
    mpv_set_option_string(mpv, "ytdl", "no");             // We handle video url parsing
    mpv_set_option_string(mpv, "user-agent", "moonplayer");
    mpv_set_option_string(mpv, "cache", QByteArray::number(Settings::cacheSize).constData());
    mpv_set_option_string(mpv, "screenshot-directory", QDir::homePath().toUtf8().constData());
    mpv_set_option_string(mpv, "reset-on-next-file", "speed,video-aspect,sub-file,sub-delay,sub-visibility");
    mpv_set_option_string(mpv, "vo", Settings::vout.toUtf8().constData());
    mpv_request_log_messages(mpv, "info");

    if (Settings::aout != "auto")
        mpv_set_option_string(mpv, "ao", Settings::aout.toUtf8().constData());

    // set hardware decoding
#if defined(Q_OS_LINUX)
    if (Settings::vout == "vaapi")
        mpv_set_option_string(mpv, "hwdec", "vaapi");
    else if (Settings::vout == "vdpau")
        mpv_set_option_string(mpv, "hwdec", "vdpau");
#elif defined(Q_OS_MAC)
    mpv_set_option_string(mpv, "hwdec", "videotoolbox");
#endif

    // listen mpv event
    mpv_observe_property(mpv, 0, "duration",         MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "width",            MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "height",           MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "playback-time",    MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "paused-for-cache", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "core-idle",        MPV_FORMAT_FLAG);
    mpv_set_wakeup_callback(mpv, postEvent, this);

    // initialize mpv
    if (mpv_initialize(mpv) < 0)
    {
        qDebug("Cannot initialize mpv.");
        exit(-1);
    }

    // catch message
    msgLabel = new QLabel(this);
    msgLabel->move(0, 0);
    msgLabel->resize(QSize(400, 30));
    msgLabel->hide();

    // create menu
    QMenu *ratio_menu = new QMenu(tr("Ratio"));
    ratio_menu->addAction("4:3", this, SLOT(setRatio_4_3()));
    ratio_menu->addAction("16:9", this, SLOT(setRatio_16_9()));
    ratio_menu->addAction("16:10", this, SLOT(setRatio_16_10()));
    ratio_menu->addAction(tr("Default"), this, SLOT(setRatio_0()));

    QMenu *speed_menu = new QMenu(tr("Speed"));
    speed_menu->addAction(tr("Speed up"), this, SLOT(speedUp()), QKeySequence("Ctrl+Right"));
    speed_menu->addAction(tr("Speed down"), this, SLOT(speedDown()), QKeySequence("Ctrl+Left"));
    speed_menu->addAction(tr("Default"), this, SLOT(speedSetToDefault()), QKeySequence("R"));

    menu = new QMenu(this);
    menu->addMenu(ratio_menu);
    menu->addMenu(speed_menu);

    menu->addAction(tr("Danmaku"), this, SLOT(switchDanmaku()), QKeySequence("D"));
    menu->addSeparator();
    menu->addAction(tr("Screenshot"), this, SLOT(screenShot()), QKeySequence("S"));
    menu->addAction(tr("Cut video"), this, SIGNAL(cutVideo()), QKeySequence("C"));
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &PlayerCore::customContextMenuRequested, this, &PlayerCore::showMenu);

    // create danmaku loader
    danmakuLoader = new DanmakuLoader(this);
    connect(danmakuLoader, &DanmakuLoader::finished, this, &PlayerCore::loadAss);

    // set state
    state = STOPPING;
    no_emit_stopped = false;
    reload_when_idle = false;
    emit_stopped_when_idle = false;
    unseekable_forced = false;

    // read unfinished_time
    QString filename = QDir(Settings::userPath).filePath("unfinished.txt");
    QFile file(filename);
    if (file.open(QFile::ReadOnly))
    {
        QByteArray data = file.readAll();
        file.close();
        if (!data.isEmpty())
        {
            QStringList list = QString::fromUtf8(data).split('\n');
            for (int i = 0; i < list.size(); i += 2)
                unfinished_time[list[i]] = list[i + 1].toInt();
        }
    }
    player_core = this;
}

PlayerCore::~PlayerCore()
{
    player_core = NULL;
    if (mpv)
    {
        mpv_terminate_destroy(mpv);
        mpv = NULL;
    }

    // save unfinished time
    if (!unfinished_time.isEmpty() && Settings::rememberUnfinished)
    {
        QByteArray data;
        QHash<QString, int64_t>::const_iterator i = unfinished_time.constBegin();
        while (i != unfinished_time.constEnd())
        {
            QString name = i.key();
            if (!name.startsWith("http://"))
                data += name.toUtf8() + '\n' + QByteArray::number((int) i.value()) + '\n';
            i++;
        }
        data.chop(1); // Remove last '\n'
        if (data.isEmpty())
            return;
        QString filename = QDir(Settings::userPath).filePath("unfinished.txt");
        QFile file(filename);
        if (!file.open(QFile::WriteOnly))
            return;
        file.write(data);
        file.close();
    }
    else
    {
        QDir dir(Settings::userPath);
        if (dir.exists("unfinished.txt"))
            dir.remove("unfinished.txt");
    }
}


// handle event
bool PlayerCore::event(QEvent *e)
{
    if (e->type() != QEvent::User)
        return QWidget::event(e);

    while (mpv)
    {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event == NULL)
            break;
        if (event->event_id == MPV_EVENT_NONE)
            break;

        handleMpvError(event->error);

        switch (event->event_id)
        {
        case MPV_EVENT_START_FILE:
            videoWidth = videoHeight = 0;
            time = 0;
            emit timeChanged(time);
            if (openfile_called)
                openfile_called = false;
            else // mpv opens file itself
            {
                char *filename = mpv_get_property_string(mpv, "filename");
                char *path = mpv_get_property_string(mpv, "path");
                file = QString::fromUtf8(path);
                danmaku.clear();
                emit newFile(QString::fromUtf8(filename), file);
                mpv_free(filename);
                mpv_free(path);
            }
            break;

        case MPV_EVENT_FILE_LOADED:
        {
            msgLabel->hide();
            int f = 0;
            handleMpvError(mpv_set_property_async(mpv, 2, "pause", MPV_FORMAT_FLAG, &f));
        }
        case MPV_EVENT_UNPAUSE:
            state = VIDEO_PLAYING;
            emit played();
            break;

        case MPV_EVENT_PAUSE:
            state = VIDEO_PAUSING;
            emit paused();
            break;

        case MPV_EVENT_END_FILE:
        {
            mpv_event_end_file *ef = static_cast<mpv_event_end_file*>(event->data);
            if (ef->error == MPV_ERROR_LOADING_FAILED)
            {
                reload_when_idle = (bool) QMessageBox::question(this, "MPV Error",
                                      tr("Fails to load: ") + file,
                                      tr("Skip"),
                                      tr("Try again"));
                if (reload_when_idle)
                {
                    state = STOPPING;
                    break;
                }
            }
            else
                handleMpvError(ef->error);
            if (no_emit_stopped)  // switch to new file when playing
                no_emit_stopped = false;
            else
            {
                if (time > length - 5)
                    unfinished_time.remove(file);
                state = STOPPING;
                emit_stopped_when_idle = true;
            }
            break;
        }
        case MPV_EVENT_IDLE:
            if (reload_when_idle)
            {
                reload_when_idle = false;
                openFile(file, danmaku);
            }
            else if (emit_stopped_when_idle)
            {
                emit_stopped_when_idle = false;
                emit stopped();
            }
            break;

        case MPV_EVENT_LOG_MESSAGE:
        {
            mpv_event_log_message *msg = static_cast<mpv_event_log_message*>(event->data);
            if (msg && msgLabel->isVisible())
                msgLabel->setText(QString::fromUtf8(msg->text));
            if (msg->log_level < MPV_LOG_LEVEL_INFO)
                fprintf(stderr, "[%s] %s", msg->prefix, msg->text);
            break;
        }
        case MPV_EVENT_PROPERTY_CHANGE:
        {
            mpv_event_property *prop = (mpv_event_property*) event->data;
            if (prop->data == NULL)
                break;
            QByteArray propName = prop->name;
            if (propName == "playback-time")
            {
                int newTime = *(double*) prop->data;
                if (newTime != time)
                {
                    time = newTime;
                    emit timeChanged(time);
                }
            }
            else if (propName == "duration")
            {
                length = *(double*) prop->data;
                emit lengthChanged(length);
                if (unfinished_time.contains(file) && !unseekable_forced)
                    setProgress(unfinished_time[file]);
            }
            else if (propName == "width")
            {
                videoWidth = *(int64_t*) prop->data;
                if (videoWidth && videoHeight)
                {
                    emit sizeChanged(QSize(videoWidth, videoHeight));
                    loadDanmaku();
                }
            }
            else if (propName == "height")
            {
                videoHeight = *(int64_t*) prop->data;
                if (videoWidth && videoHeight)
                {
                    emit sizeChanged(QSize(videoWidth, videoHeight));
                    loadDanmaku();
                }
            }
            else if (propName == "paused-for-cache")
            {
                if (prop->format == MPV_FORMAT_FLAG)
                {
                    if ((bool)*(unsigned*)prop->data && state != STOPPING)
                        showText("Network is slow...");
                    else
                        showText("");
                }
            }
            else if (propName == "core-idle")
            {
                if(prop->format == MPV_FORMAT_FLAG)
                {
                    if( *(unsigned*)prop->data && state == VIDEO_PLAYING)
                        showText("Buffering...");
                    else
                        showText("");
                }
            }
            break;
        }
        default: break;
        }
    }
    return true;
}

void PlayerCore::mouseDoubleClickEvent(QMouseEvent *e)
{
    if (e->buttons() == Qt::LeftButton)
        emit fullScreen();
}

void PlayerCore::showMenu(const QPoint&)
{
    menu->exec(QCursor::pos());
}

// open file
void PlayerCore::openFile(const QString &file, const QString &danmaku)
{
    if (state != STOPPING)
    {
        no_emit_stopped = true;
        if (time <= length - 5 && Settings::rememberUnfinished)
            unfinished_time[this->file] = time;
        else if (time > length - 5)
            unfinished_time.remove(file);
    }

    this->file = file;
    this->danmaku = danmaku;
    if (danmaku.isEmpty() && !file.startsWith("http://") && !file.startsWith("https://"))
    {
        //get danmaku's url of local videos
        QFile f(file + ".danmaku");
        if (f.open(QFile::ReadOnly))
        {
            this->danmaku = QString::fromUtf8(f.readAll());
            f.close();
        }
    }

    // set referer and seekability
    if (file.startsWith("http:") || file.startsWith("https:"))
    {
        QString host = QUrl(file).host();
        if (referer_table.contains(host))
            handleMpvError(mpv_set_option_string(mpv, "referrer", referer_table[host].constData()));
        else
            handleMpvError(mpv_set_option_string(mpv, "referrer", ""));

        /* Some websites does not allow "Range" option in http request header.
         * To hack these websites, we force ffmpeg/libav to set the stream unseekable.
         * Then we make the video seekable again by enabling seeking in cache.
         */
        if (unseekable_hosts.contains(host))
        {
            handleMpvError(mpv_set_option_string(mpv, "stream-lavf-o", "seekable=0"));
            handleMpvError(mpv_set_option_string(mpv, "force-seekable", "yes"));
            unseekable_forced = true;
        }
        else
        {
            handleMpvError(mpv_set_option_string(mpv, "stream-lavf-o", ""));
            handleMpvError(mpv_set_option_string(mpv, "force-seekable", "no"));
            unseekable_forced = false;
        }
    }
    else
    {
        handleMpvError(mpv_set_option_string(mpv, "stream-lavf-o", ""));
        handleMpvError(mpv_set_option_string(mpv, "force-seekable", "no"));
        unseekable_forced = false;
    }

    msgLabel->show();
    speed = 1.0;
    danmaku_visible = true;

    openfile_called = true;
    QByteArray tmp = file.toUtf8();
    const char *args[] = {"loadfile", tmp.constData(), NULL};
    handleMpvError(mpv_command_async(mpv, 2, args));
}

// switch between play and pause
void PlayerCore::changeState()
{
    int f;
    switch (state)
    {
    case VIDEO_PAUSING:
        f = 0;
        handleMpvError(mpv_set_property_async(mpv, 2, "pause", MPV_FORMAT_FLAG, &f));
        break;
    case VIDEO_PLAYING:
        f = 1;
        handleMpvError(mpv_set_property_async(mpv, 2, "pause", MPV_FORMAT_FLAG, &f));
        break;
    default: break;
    }
}

void PlayerCore::stop()
{
    if (state == STOPPING)
        return;
    const char *args[] = {"stop", NULL};
    handleMpvError(mpv_command_async(mpv, 0, args));
    if (time < length - 2 && Settings::rememberUnfinished)
        unfinished_time[file] = time;
}

void PlayerCore::setVolume(int volume)
{
    double vol = volume * 10.0;
    if (state == STOPPING)
        mpv_set_option(mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
    else
    {
        mpv_set_property_async(mpv, 2, "volume", MPV_FORMAT_DOUBLE, &vol);
        showText("Volume: " + QByteArray::number(vol));
    }
}

void PlayerCore::setProgress(int pos)
{
    if (state == STOPPING)
        return;
    if (pos != time)
    {
        QByteArray tmp = QByteArray::number(pos);
        const char *args[] = {"seek", tmp.constData(), "absolute", NULL};
        mpv_command_async(mpv, 2, args);
    }
}

void PlayerCore::jumpTo(int pos)
{
    if (state == STOPPING)
        return;
    if (state == VIDEO_PLAYING)
        changeState();
    setProgress(pos);
}

// danmaku
void PlayerCore::loadDanmaku()
{
    if (!danmaku.isEmpty() && state != STOPPING)
    {
        if (danmaku.contains(" http"))
            danmakuLoader->load(danmaku.section(' ', 1), videoWidth, videoHeight); //danmaku has delay
        else
            danmakuLoader->load(danmaku, videoWidth, videoHeight);
    }
}

void PlayerCore::loadAss(const QString &assFile)
{
    if (state == STOPPING)
        return;
    QByteArray tmp = assFile.toUtf8();
    const char *args[] = {"sub-add", tmp.constData(), "select", NULL};
    handleMpvError(mpv_command_async(mpv, 2, args));
    if (danmaku.contains(" http")) // has delay
    {
        double delay = - danmaku.section(' ', 0, 0).toDouble();
        handleMpvError(mpv_set_property_async(mpv, 2, "sub-delay", MPV_FORMAT_DOUBLE, &delay));
    }
}

void PlayerCore::switchDanmaku()
{
    if (state == STOPPING || danmaku.isEmpty())
        return;
    danmaku_visible = !danmaku_visible;
    handleMpvError(mpv_set_property_async(mpv, 0, "sub-visibility", MPV_FORMAT_FLAG, &danmaku_visible));
}

void PlayerCore::screenShot()
{
    if (state == STOPPING)
        return;
    const char *args[] = {"osd-msg" ,"screenshot", NULL};
    mpv_command_async(mpv, 2, args);
}

// set playback speed
void PlayerCore::speedDown()
{
    if (speed > 0.5 && state != STOPPING)
    {
        speed -= 0.1;
        mpv_set_property_async(mpv, 2, "speed", MPV_FORMAT_DOUBLE, &speed);
        showText("Speed: " + QByteArray::number(speed));
    }
}

void PlayerCore::speedUp()
{
    if (speed < 2.0 && state != STOPPING)
    {
        speed += 0.1;
        mpv_set_property_async(mpv, 2, "speed", MPV_FORMAT_DOUBLE, &speed);
        showText("Speed: " + QByteArray::number(speed));
    }
}

void PlayerCore::speedSetToDefault()
{
    if (state != STOPPING)
    {
        speed = 1;
        mpv_set_property_async(mpv, 2, "speed", MPV_FORMAT_DOUBLE, &speed);
        showText("Speed: 1");
    }
}

// set video aspect
void PlayerCore::setRatio_0()
{
    if (state == STOPPING)
        return;
    mpv_set_property_string(mpv, "video-aspect", "0");
}

void PlayerCore::setRatio_4_3()
{
    if (state == STOPPING)
        return;
    mpv_set_property_string(mpv, "video-aspect", "4:3");
}

void PlayerCore::setRatio_16_9()
{
    if (state == STOPPING)
        return;
    mpv_set_property_string(mpv, "video-aspect", "16:9");
}

void PlayerCore::setRatio_16_10()
{
    if (state == STOPPING)
        return;
    mpv_set_property_string(mpv, "video-aspect", "16:10");
}

// handle error
void PlayerCore::handleMpvError(int code)
{
    if(code >= 0)
        return;
    QMessageBox::warning(this, "MPV Error", "Error while playing file:\n" + file +
                         "\n\nMPV Error: " + mpv_error_string(code));
}

// show text
void PlayerCore::showText(const QByteArray &text)
{
    if (state == STOPPING)
        return;
    const char *args[] = {"show-text", text.constData(), NULL};
    mpv_command_async(mpv, 2, args);
}
