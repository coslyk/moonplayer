#include "playercore.h"
#include "danmakuloader.h"
#include "platforms.h"
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
#include <QMessageBox>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QTimer>

static void postEvent(void *ptr)
{
    PlayerCore *core = (PlayerCore*) ptr;
    QCoreApplication::postEvent(core, new QEvent(QEvent::User));
}


static void *get_proc_address(void *, const char *name)
{
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return NULL;
    return (void*) glctx->getProcAddress(name);
}


PlayerCore *player_core = NULL;

static QHash<QString,int64_t> unfinished_time;

PlayerCore::PlayerCore(QWidget *parent) :
    QOpenGLWidget(parent)
{
    printf("Initialize mpv backend...\n");
    setFocusPolicy(Qt::StrongFocus);

    // create mpv instance
    mpv = mpv_create();
    if (!mpv)
    {
        qDebug("Cannot create mpv instance.");
        exit(-1);
    }

    // set mpv options
    mpv_set_option_string(mpv, "softvol", "yes");         // mpv handles the volume
    mpv_set_option_string(mpv, "input-cursor", "no");     // We handle cursor
    mpv_set_option_string(mpv, "cursor-autohide", "no");
    mpv_set_option_string(mpv, "input-default-bindings", "no");
    mpv_set_option_string(mpv, "input-vo-keyboard", "no");
    mpv_set_option_string(mpv, "osc", "no");
    mpv_set_option_string(mpv, "ytdl", "no");             // We handle video url parsing
    mpv_set_option_string(mpv, "cache", QByteArray::number(Settings::cacheSize).constData());
    mpv_set_option_string(mpv, "screenshot-directory", QDir::homePath().toUtf8().constData());
    mpv_set_option_string(mpv, "reset-on-next-file", "speed,video-aspect,sub-file,sub-delay,sub-visibility");
    mpv_set_option_string(mpv, "vo", "opengl-cb");
    mpv_request_log_messages(mpv, "warn");

    if (Settings::aout != "auto")
        mpv_set_option_string(mpv, "ao", Settings::aout.toUtf8().constData());

    // set hardware decoding
#if defined(Q_OS_LINUX)
    if (Settings::hwdec == "auto")
        mpv_set_option_string(mpv, "opengl-hwdec-interop", "auto");
    else if (Settings::hwdec == "vaapi")
        mpv_set_option_string(mpv, "opengl-hwdec-interop", "vaapi-egl");
    else
        mpv_set_option_string(mpv, "opengl-hwdec-interop", "vdpau-glx");
    mpv_set_option_string(mpv, "hwdec", Settings::hwdec.toUtf8().constData());
#elif defined(Q_OS_MAC)
    mpv_set_option_string(mpv, "opengl-hwdec-interop", "videotoolbox");
    mpv_set_option_string(mpv, "hwdec", "videotoolbox");
#elif defined(Q_OS_WIN)
    mpv_set_option_string(mpv, "opengl-backend", "angle");
    mpv_set_option_string(mpv, "hwdec", "d3d11va");
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
        exit(EXIT_FAILURE);
    }

    // initialize opengl
    mpv_gl = (mpv_opengl_cb_context*) mpv_get_sub_api(mpv, MPV_SUB_API_OPENGL_CB);
    if (!mpv_gl)
    {
        qDebug("OpenGL not compiled in");
        exit(EXIT_FAILURE);
    }
    mpv_opengl_cb_set_update_callback(mpv_gl, PlayerCore::on_update, (void*) this);
    connect(this, &PlayerCore::frameSwapped, this, &PlayerCore::swapped);

    // create danmaku loader
    danmakuLoader = new DanmakuLoader(this);
    connect(danmakuLoader, &DanmakuLoader::finished, this, &PlayerCore::loadAss);

    // set state
    state = STOPPING;
    no_emit_stopped = false;
    reload_when_idle = false;
    emit_stopped_when_idle = false;
    unseekable_forced = false;
    rendering_paused = false;

    // read unfinished_time
    QString filename = QDir(getUserPath()).filePath("unfinished.txt");
    QFile file(filename);
    if (file.open(QFile::ReadOnly | QFile::Text))
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

// opengl
void PlayerCore::initializeGL()
{
    printf("OpenGL Version: %i.%i\n", context()->format().majorVersion(), context()->format().minorVersion());
    int r = mpv_opengl_cb_init_gl(mpv_gl, NULL, get_proc_address, NULL);
    if (r < 0)
    {
        qDebug("Cannot initialize OpenGL.");
        exit(EXIT_FAILURE);
    }
}

void PlayerCore::paintGL()
{
    mpv_opengl_cb_draw(mpv_gl, defaultFramebufferObject(), width(), -height());
}

void PlayerCore::swapped()
{
    mpv_opengl_cb_report_flip(mpv_gl, 0);
}

void PlayerCore::maybeUpdate()
{
    if (window()->isMinimized() || rendering_paused)
    {
        makeCurrent();
        paintGL();
        context()->swapBuffers(context()->surface());
        swapped();
        doneCurrent();
    }
    else
        update();
}

void PlayerCore::on_update(void *ctx)
{
    QMetaObject::invokeMethod((PlayerCore*) ctx, "maybeUpdate");
}

void PlayerCore::pauseRendering()
{
    rendering_paused = true;
}

void PlayerCore::unpauseRendering()
{
    rendering_paused = false;
}


PlayerCore::~PlayerCore()
{
    player_core = NULL;
    makeCurrent();
    if (mpv_gl)
        mpv_opengl_cb_set_update_callback(mpv_gl, NULL, NULL);
    mpv_opengl_cb_uninit_gl(mpv_gl);
    mpv_gl = NULL;
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
        QString filename = QDir(getUserPath()).filePath("unfinished.txt");
        QFile file(filename);
        if (!file.open(QFile::WriteOnly | QFile::Text))
            return;
        file.write(data);
        file.close();
    }
    else
    {
        QDir dir(getUserPath());
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
            break;

        case MPV_EVENT_FILE_LOADED:
        {
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

    // set referer, user-agent and seekability
    if (file.startsWith("http:") || file.startsWith("https:"))
    {
        QString host = QUrl(file).host();
        if (referer_table.contains(host))
            handleMpvError(mpv_set_option_string(mpv, "referrer", referer_table[host].constData()));
        else
            handleMpvError(mpv_set_option_string(mpv, "referrer", ""));
        // set user-agent
        handleMpvError(mpv_set_option_string(mpv, "user-agent", generateUA(file)));

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

    speed = 1.0;
    danmaku_visible = true;

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
