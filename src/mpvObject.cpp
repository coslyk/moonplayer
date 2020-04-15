#include "mpvObject.h"

#include <stdexcept>
#include <clocale>

#include <QOpenGLContext>
#include <QGuiApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QMetaType>

#include <QtGui/QOpenGLFramebufferObject>

#include <QtQuick/QQuickWindow>
#include <QtQuick/QQuickView>

#include "accessManager.h"
#include "danmakuLoader.h"


// Workaround for some gl.h headers
#ifndef GLAPIENTRY
#ifdef APIENTRY
#define GLAPIENTRY APIENTRY
#elif defined(Q_OS_WIN)
#define GLAPIENTRY __stdcall
#else
#define GLAPIENTRY
#endif
#endif

// Linux display server
#ifdef Q_OS_LINUX
#include <QGuiApplication>
#include <QX11Info>
#include <qpa/qplatformnativeinterface.h>
#endif

/* MPV Renderer */

static void *get_proc_address_mpv(void *ctx, const char *name) {
    Q_UNUSED(ctx)
    
    QOpenGLContext *glctx = QOpenGLContext::currentContext();
    if (!glctx)
        return nullptr;
    return reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name)));
}


class MpvRenderer : public QQuickFramebufferObject::Renderer
{
    MpvObject *obj;
    
public:
    MpvRenderer(MpvObject *obj) : obj(obj)
    {
        mpv_set_wakeup_callback(obj->mpv, [](void *ctx) {
            MpvObject *obj = reinterpret_cast<MpvObject*>(ctx);
            QMetaObject::invokeMethod(obj, "onMpvEvent", Qt::QueuedConnection);
        }, obj);
    }

    virtual ~MpvRenderer()
    {
    }
    
    // This function is called when a new FBO is needed.
    // This happens on the initial frame.
    QOpenGLFramebufferObject * createFramebufferObject(const QSize & size)
    {
        // init mpv_gl
        if (obj->mpv_gl == nullptr)
        {
            mpv_opengl_init_params gl_init_params{get_proc_address_mpv, nullptr, nullptr};
            mpv_render_param params[]{
                {MPV_RENDER_PARAM_API_TYPE, const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL)},
                {MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
                {MPV_RENDER_PARAM_INVALID, nullptr},  // Placeholder for Linux display parameters
                {MPV_RENDER_PARAM_INVALID, nullptr}
            };
        
            // Set Linux display
#ifdef Q_OS_LINUX
            if (QX11Info::isPlatformX11())  // X11
            {
                params[2] = {
                    .type = MPV_RENDER_PARAM_X11_DISPLAY,
                    .data = QX11Info::display()
                };
            } else {    // Wayland
                params[2] = {
                    .type = MPV_RENDER_PARAM_WL_DISPLAY,
                    .data = QGuiApplication::platformNativeInterface()->nativeResourceForWindow("display", NULL)
                };
            }
#endif
        
            if (mpv_render_context_create(&obj->mpv_gl, obj->mpv, params) < 0)
                throw std::runtime_error("failed to initialize mpv GL context");
            mpv_render_context_set_update_callback(obj->mpv_gl, [](void *ctx) {
                MpvObject *obj = reinterpret_cast<MpvObject*>(ctx);
                QMetaObject::invokeMethod(obj, "update", Qt::QueuedConnection);
            }, obj);
        }
        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }

    void render()
    {
        obj->window()->resetOpenGLState();

        QOpenGLFramebufferObject *fbo = framebufferObject();
        mpv_opengl_fbo mpfbo {
            .fbo = static_cast<int>(fbo->handle()),
            .w = fbo->width(),
            .h = fbo->height(),
            .internal_format = 0};
        int flip_y = 0;

        mpv_render_param params[] = {
            // Specify the default framebuffer (0) as target. This will
            // render onto the entire screen. If you want to show the video
            // in a smaller rectangle or apply fancy transformations, you'll
            // need to render into a separate FBO and draw it manually.
            {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
            // Flip rendering (needed due to flipped GL coordinate system).
            {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
            {MPV_RENDER_PARAM_INVALID, nullptr}
        };
        // See render_gl.h on what OpenGL environment mpv expects, and
        // other API details.
        mpv_render_context_render(obj->mpv_gl, params);

        obj->window()->resetOpenGLState();
    }
};


MpvObject* MpvObject::s_instance = nullptr;

MpvObject::MpvObject(QQuickItem * parent) :
    QQuickFramebufferObject(parent),
    mpv_gl(nullptr),
    m_state(STOPPED),
    m_subVisible(true),
    m_videoWidth(0),
    m_videoHeight(0),
    m_speed(1)
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = this;
    
    no_emit_stopped = emit_stopped_when_idle = false;
    m_stopByUser = false;
    m_time = m_duration = 0;
    m_volume = 100;
    
    // Access settings
    QSettings settings;
    
    mpv = mpv::qt::Handle::FromRawHandle(mpv_create());
    if (!mpv)
        throw std::runtime_error("could not create mpv context");

    // set mpv options
    mpv_set_option_string(mpv, "softvol", "yes");         // mpv handles the volume
    mpv_set_option_string(mpv, "ytdl", "no");             // We handle video url parsing
    mpv_set_option_string(mpv, "screenshot-directory", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation).toUtf8().constData());
    mpv_set_option_string(mpv, "reset-on-next-file", "speed,video-aspect,af,sub-visibility,audio-delay");
    
    mpv_observe_property(mpv, 0, "duration",         MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "width",            MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "height",           MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "playback-time",    MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "paused-for-cache", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "core-idle",        MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "track-list",       MPV_FORMAT_NODE);
    mpv_request_log_messages(mpv, "warn");
    
    
    // Configure hardware decoding
    bool hwdecCopy = settings.value("video/hwdec_copy_mode", false).toBool();
    
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    Hwdec hwdec = (Hwdec) settings.value("video/hwdec", 0).toInt();
    switch (hwdec)
    {
        case AUTO:
            mpv_set_option_string(mpv, "hwdec-preload", "auto");
            mpv_set_option_string(mpv, "opengl-hwdec-interop", "auto");
            mpv_set_option_string(mpv, "hwdec", hwdecCopy ? "auto-copy" : "auto");
            break;
        case VAAPI:
            mpv_set_option_string(mpv, "hwdec-preload", "vaapi-egl");
            mpv_set_option_string(mpv, "opengl-hwdec-interop", "vaapi-egl");
            mpv_set_option_string(mpv, "hwdec", hwdecCopy ? "vaapi-copy" : "vaapi");
            break;
        case VDPAU:
            mpv_set_option_string(mpv, "hwdec-preload", "vdpau-glx");
            mpv_set_option_string(mpv, "opengl-hwdec-interop", "vdpau-glx");
            mpv_set_option_string(mpv, "hwdec", hwdecCopy ? "vdpau-copy" : "vdpau");
            break;
        default: break;
    }
    
#elif defined(Q_OS_MAC)
    mpv_set_option_string(mpv, "opengl-hwdec-interop", "videotoolbox");
    mpv_set_option_string(mpv, "hwdec", hwdecCopy ? "videotoolbox-copy" : "videotoolbox");
    
#elif defined(Q_OS_WIN)
    mpv_set_option_string(mpv, "gpu-context", "angle");
    if (QSysInfo::productVersion() == "8.1" && QSysInfo::productVersion() == "10")
        mpv_set_option_string(mpv, "hwdec", hwdecCopy ? "d3d11va-copy" : "d3d11va");
    else
        mpv_set_option_string(mpv, "hwdec", hwdecCopy ? "dxva2-copy" : "dxva2");
#endif
    
    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");
}

MpvObject::~MpvObject()
{
    if (mpv_gl)
        mpv_render_context_free(mpv_gl);
    //mpv_destroy(mpv);
}


// Open file
void MpvObject::open(const QUrl& fileUrl, const QUrl& danmakuUrl, const QUrl& audioTrack)
{
    if (m_state != STOPPED)
        no_emit_stopped = true;
    
    QStringList args;
    
    // set network parameters
    if (!fileUrl.isLocalFile())
    {
        // set referer
        mpv_set_option_string(mpv, "referrer", NetworkAccessManager::instance()->refererOf(fileUrl));

        // set user-agent
        mpv_set_option_string(mpv, "user-agent", NetworkAccessManager::instance()->userAgentOf(fileUrl));

        /* Some websites does not allow "Range" option in http request header.
         * To hack these websites, we force ffmpeg/libav to set the stream unseekable.
         * Then we make the video seekable again by enabling seeking in cache.
         */
        if (NetworkAccessManager::instance()->urlIsUnseekable(fileUrl))
        {
            mpv_set_option_string(mpv, "stream-lavf-o", "seekable=0");
            mpv_set_option_string(mpv, "force-seekable", "yes");
        }
        else
        {
            mpv_set_option_string(mpv, "stream-lavf-o", "");
            mpv_set_option_string(mpv, "force-seekable", "no");
        }
    }
    else
    {
        mpv_set_option_string(mpv, "stream-lavf-o", "");
        mpv_set_option_string(mpv, "force-seekable", "no");
    }
    
    args << "loadfile" << (fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.toString());
    command(args);
    m_danmakuUrl = danmakuUrl;
    m_audioToBeAdded = audioTrack;
}


// Play, Pause, Stop & Get state
void MpvObject::play()
{
    if (m_state == VIDEO_PAUSED)
    {
        setProperty("pause", false);
    }
}

void MpvObject::pause()
{
    if (m_state == VIDEO_PLAYING)
    {
        setProperty("pause", true);
    }
}

void MpvObject::stop()
{
    if (m_state != STOPPED)
    {
        QStringList args;
        args << "stop";
        command(args);
        m_stopByUser = true;
    }
}

// Seek
void MpvObject::seek ( qint64 time, bool absolute )
{
    if (m_state != STOPPED && time != m_time)
    {
        QStringList args;
        args << "seek" << QString::number(time) << (absolute ? "absolute" : "relative");
        command(args);
    }
}


// Set volume
void MpvObject::setVolume(int volume)
{
    if (m_volume == volume)
        return;
    
    double vol = volume;
    if (m_state == STOPPED)
    {
        mpv_set_option(mpv, "volume", MPV_FORMAT_DOUBLE, &vol);
    }
    else
    {
        setProperty("volume", vol);
        showText("Volume: " + QString::number(volume));
    }
    
    m_volume = volume;
    emit volumeChanged();
}

// Set subtitle visibility
void MpvObject::setSubVisible(bool subVisible)
{
    if (m_subVisible == subVisible)
        return;
    m_subVisible = subVisible;
    if (m_state == STOPPED)
        mpv_set_option(mpv, "sub-visibility", MPV_FORMAT_FLAG, &m_subVisible);
    else
        setProperty("sub-visibility", m_subVisible);
    emit subVisibleChanged();
}

// Set speed
void MpvObject::setSpeed(double speed)
{
    if (m_speed - speed < 0.1 && speed - m_speed < 0.1)
        return;
    m_speed = speed;
    if (m_state == STOPPED)
        mpv_set_option(mpv, "speed", MPV_FORMAT_DOUBLE, &m_speed);
    else
        setProperty("speed", m_speed);
    showText("Speed: " + QString::number(m_speed));
    emit speedChanged();
}


// Add audio track
void MpvObject::addAudioTrack(const QUrl& url)
{
    if (m_state == STOPPED)
        return;
    QStringList args;
    args << "audio-add" << (url.isLocalFile() ? url.toLocalFile() : url.toString()) << "select";
    command(args);
}

// Add subtitle
void MpvObject::addSubtitle(const QUrl& url)
{
    if (m_state == STOPPED)
        return;
    QStringList args;
    args << "sub-add" << (url.isLocalFile() ? url.toLocalFile() : url.toString()) << "select";
    command(args);
}




// Take screenshot
void MpvObject::screenshot()
{
    if (m_state == STOPPED)
        return;
    QStringList args;
    args << "osd-msg" << "screenshot";
    command(args);
}


void MpvObject::onMpvEvent()
{
    while (mpv)
    {
        mpv_event *event = mpv_wait_event(mpv, 0);
        if (event == NULL)
            break;
        if (event->event_id == MPV_EVENT_NONE)
            break;

        switch (event->event_id)
        {
        case MPV_EVENT_START_FILE:
            m_videoWidth = m_videoHeight = 0;    // Set videoSize invalid
            m_time = 0;
            m_subVisible = true;
            m_speed = 1;
            emit timeChanged();
            emit subVisibleChanged();
            emit speedChanged();

            break;

        case MPV_EVENT_FILE_LOADED:
            setProperty("pause", false);
            // go through
            
        case MPV_EVENT_UNPAUSE:
            m_state = VIDEO_PLAYING;
            emit stateChanged();
            break;

        case MPV_EVENT_PAUSE:
            m_state = VIDEO_PAUSED;
            emit stateChanged();
            break;

        case MPV_EVENT_END_FILE:
        {
            mpv_event_end_file *ef = static_cast<mpv_event_end_file*>(event->data);
            handleMpvError(ef->error);
            if (no_emit_stopped)  // switch to new file when playing
                no_emit_stopped = false;
            else
            {
                m_state = STOPPED;
                emit_stopped_when_idle = true;
            }
            break;
        }

        case MPV_EVENT_IDLE:
            if (emit_stopped_when_idle)
            {
                emit_stopped_when_idle = false;
                emit stateChanged();
                emit stopped(m_stopByUser);
                m_stopByUser = false;
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

            if (prop->data == nullptr)
                break;

            QByteArray propName = prop->name;

            if (propName == "playback-time")
            {
                qint64 newTime = *(double*) prop->data;
                if (newTime != m_time)
                {
                    m_time = newTime;
                    emit timeChanged();
                }
            }

            else if (propName == "duration")
            {
                m_duration = *(double*) prop->data;
                emit durationChanged();
            }

            else if (propName == "width")
            {
                if (!m_audioToBeAdded.isEmpty())
                {
                    addAudioTrack(m_audioToBeAdded);
                    m_audioToBeAdded = QUrl();
                }
                m_videoWidth = *(int64_t*) prop->data;
                if (m_videoWidth && m_videoHeight)  // videoSize is valid?
                {
                    emit videoSizeChanged();
                    // Load danmaku
                    if (!m_danmakuUrl.isEmpty())
                        DanmakuLoader::instance()->start(m_danmakuUrl, m_videoWidth, m_videoHeight);
                }
            }

            else if (propName == "height")
            {
                m_videoHeight = *(int64_t*) prop->data;
                if (m_videoWidth && m_videoHeight)
                {
                    emit videoSizeChanged();
                    if (!m_danmakuUrl.isEmpty())
                        DanmakuLoader::instance()->start(m_danmakuUrl, m_videoWidth, m_videoHeight);
                }
            }

            else if (propName == "paused-for-cache")
            {
                if (prop->format == MPV_FORMAT_FLAG)
                {
                    if ((bool)*(unsigned*)prop->data && m_state != STOPPED)
                        showText("Network is slow...");
                    else
                        showText("");
                }
            }

            else if (propName == "core-idle")
            {
                if(prop->format == MPV_FORMAT_FLAG)
                {
                    if( *(unsigned*)prop->data && m_state == VIDEO_PLAYING)
                        showText("Buffering...");
                    else
                        showText("");
                }
            }
            
            else if (propName == "track-list") // Read tracks info
            {
                m_subtitles.clear();
                m_audioTracks.clear();
                mpv_node* node = static_cast<mpv_node*>(prop->data);
                QVariantList trackList = mpv::qt::node_to_variant(node).toList();
                foreach (QVariant i, trackList)
                {
                    QVariantHash item = i.toHash();
                    if (item["type"].toString() == "sub")  // Subtitles
                    {
                        int id = item["id"].toInt();
                        QString title = item["title"].toString();
                        if (m_subtitles.count() <= id)
                        {
                            for (int j = m_subtitles.count(); j < id; j++)
                                m_subtitles.append('#' + QString::number(j));
                            m_subtitles.append(title.isEmpty() ? '#' + QString::number(id) : title);
                        }
                        else
                        {
                            m_subtitles[id] = title.isEmpty() ? '#' + QString::number(id) : title;
                        }
                    }

                    else if (item["type"].toString() == "audio")  // Audio tracks
                    {
                        int id = item["id"].toInt();
                        QString title = item["title"].toString();
                        if (m_audioTracks.count() <= id)
                        {
                            for (int j = m_audioTracks.count(); j < id; j++)
                                m_audioTracks.append('#' + QString::number(j));
                            m_audioTracks.append(title.isEmpty() ? '#' + QString::number(id) : title);
                        }
                        else
                        {
                            m_audioTracks[id] = title.isEmpty() ? '#' + QString::number(id) : title;
                        }
                    }
                }
                emit subtitlesChanged();
                emit audioTracksChanged();
            }
            break;
        }
        default: break;
        }
    }
}



void MpvObject::command(const QStringList& params)
{
    // convert QStringList => array of char*
    const char **args = (const char**) calloc(params.length() + 1, sizeof(char*));
    for (int i = 0; i < params.length(); i++)
    {
        args[i] = strdup(params[i].toUtf8().constData());
    }
    args[params.length()] = nullptr;
    
    // command
    mpv_command_async(mpv, 2, args);
    
    // free
    for (int i = 0; i < params.length(); i++)
    {
        free((void*) args[i]);
    }
    free(args);
}

// set property
void MpvObject::setProperty(const QString& name, const QVariant& value)
{
    int retVal;
    switch ((int) value.type())
    {
        case (int) QMetaType::Bool:
        {
            int v = value.toInt();
            retVal = mpv_set_property_async(mpv, 2, name.toUtf8().constData(), MPV_FORMAT_FLAG, &v);
            break;
        }
        case (int) QMetaType::Int:
        case (int) QMetaType::Long:
        case (int) QMetaType::LongLong:
        {
            qint64 v = value.toLongLong();
            retVal = mpv_set_property_async(mpv, 2, name.toUtf8().constData(), MPV_FORMAT_INT64, &v);
            break;
        }
        case (int) QMetaType::Float:
        case (int) QMetaType::Double:
        {
            double v = value.toDouble();
            retVal = mpv_set_property_async(mpv, 2, name.toUtf8().constData(), MPV_FORMAT_DOUBLE, &v);
            break;
        }
        case (int) QMetaType::QByteArray:
        {
            QByteArray v = value.toByteArray();
            retVal = mpv_set_property_async(mpv, 2, name.toUtf8().constData(), MPV_FORMAT_STRING, v.data());
            break;
        }
        case (int) QMetaType::QString:
        {
            QByteArray v = value.toString().toUtf8();
            retVal = mpv_set_property_async(mpv, 2, name.toUtf8().constData(), MPV_FORMAT_STRING, v.data());
            break;
        }
    }
    // Error check
    handleMpvError(retVal);
}

void MpvObject::handleMpvError ( int code )
{
    if (code < 0)
    {
        qDebug() << "MPV Error: " << mpv_error_string(code);
    }
}

void MpvObject::showText(const QString& text)
{
    QStringList args;
    args << "show-text" << text;
    command(args);
}



QQuickFramebufferObject::Renderer *MpvObject::createRenderer() const
{
    window()->setPersistentOpenGLContext(true);
    window()->setPersistentSceneGraph(true);
    return new MpvRenderer(const_cast<MpvObject*>(this));
}


