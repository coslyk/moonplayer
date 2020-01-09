#include "mpvObject.h"

#include <stdexcept>
#include <clocale>

#include <QObject>
#include <QtGlobal>
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


// workaround for some gl.h headers
#ifndef GLAPIENTRY
#ifdef APIENTRY
#define GLAPIENTRY APIENTRY
#elif defined(Q_OS_WIN)
#define GLAPIENTRY __stdcall
#else
#define GLAPIENTRY
#endif
#endif


// wayland fix
#ifdef Q_OS_LINUX
#include <QGuiApplication>
#include <QX11Info>
#include <qpa/qplatformnativeinterface.h>

static void* GLAPIENTRY glMPGetNativeDisplay(const char *name)
{
    if (strcmp(name, "wl") == 0 && !QX11Info::isPlatformX11())
        return QGuiApplication::platformNativeInterface()->nativeResourceForWindow("display", NULL);
    else if (strcmp(name, "x11") == 0 && QX11Info::isPlatformX11())
    {
        return QX11Info::display();
    }
    return NULL;
}
#endif // Q_OS_LINUX



class MpvRenderer : public QQuickFramebufferObject::Renderer
{
    static void *get_proc_address(void *ctx, const char *name) {
        Q_UNUSED(ctx)
        
#ifdef Q_OS_LINUX
        if(strcmp(name, "glMPGetNativeDisplay") == 0)
            return (void*) glMPGetNativeDisplay;
#endif
        
        QOpenGLContext *glctx = QOpenGLContext::currentContext();
        if (!glctx)
            return nullptr;
        return (void *)glctx->getProcAddress(QByteArray(name));
    }

    mpv::qt::Handle mpv;
    QQuickWindow *window;
    mpv_opengl_cb_context *mpv_gl;
public:
    MpvRenderer(const MpvObject *obj)
        : mpv(obj->mpv), window(obj->window()), mpv_gl(obj->mpv_gl)
    {
#ifdef Q_OS_LINUX
        int r = mpv_opengl_cb_init_gl(mpv_gl, "GL_MP_MPGetNativeDisplay", get_proc_address, nullptr);
#else
        int r = mpv_opengl_cb_init_gl(mpv_gl, nullptr, get_proc_address, nullptr);
#endif
        if (r < 0)
            throw std::runtime_error("could not initialize OpenGL");
    }

    virtual ~MpvRenderer()
    {
        // Until this call is done, we need to make sure the player remains
        // alive. This is done implicitly with the mpv::qt::Handle instance
        // in this class.
        mpv_opengl_cb_uninit_gl(mpv_gl);
    }

    void render()
    {
        QOpenGLFramebufferObject *fbo = framebufferObject();
        window->resetOpenGLState();
        mpv_opengl_cb_draw(mpv_gl, fbo->handle(), fbo->width(), fbo->height());
        window->resetOpenGLState();
    }
};


MpvObject* MpvObject::s_instance = nullptr;

static void postEvent(void *ptr);

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
    mpv_set_option_string(mpv, "vo", "opengl-cb");
    
    mpv_observe_property(mpv, 0, "duration",         MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "width",            MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "height",           MPV_FORMAT_INT64);
    mpv_observe_property(mpv, 0, "playback-time",    MPV_FORMAT_DOUBLE);
    mpv_observe_property(mpv, 0, "paused-for-cache", MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "core-idle",        MPV_FORMAT_FLAG);
    mpv_observe_property(mpv, 0, "track-list",       MPV_FORMAT_NODE);
    mpv_observe_property(mpv, 0, "sid",              MPV_FORMAT_INT64);
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
    
    // listen mpv event
    mpv_set_wakeup_callback(mpv, postEvent, this);
    
    if (mpv_initialize(mpv) < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Request hw decoding, just for testing.

    // Setup the callback that will make QtQuick update and redraw if there
    // is a new video frame. Use a queued connection: this makes sure the
    // doUpdate() function is run on the GUI thread.
    mpv_gl = (mpv_opengl_cb_context *)mpv_get_sub_api(mpv, MPV_SUB_API_OPENGL_CB);
    if (!mpv_gl)
        throw std::runtime_error("OpenGL not compiled in");
    mpv_opengl_cb_set_update_callback(mpv_gl, MpvObject::on_update, (void *)this);
    connect(this, &MpvObject::onUpdate, this, &MpvObject::doUpdate,
            Qt::QueuedConnection);
}

MpvObject::~MpvObject()
{
    if (mpv_gl)
        mpv_opengl_cb_set_update_callback(mpv_gl, NULL, NULL);
}

void MpvObject::on_update(void *ctx)
{
    MpvObject *self = (MpvObject *)ctx;
    emit self->onUpdate();
}

// connected to onUpdate(); signal makes sure it runs on the GUI thread
void MpvObject::doUpdate()
{
    update();
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

// Set subtitle id
void MpvObject::setSid(int sid)
{
    if (m_sid == sid)
        return;
    m_sid = sid;
    if (m_state != STOPPED && sid >= 0)
        setProperty("sid", sid);
    emit sidChanged();
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


// Set video aspect
void MpvObject::setAspect(MpvObject::Aspect aspect)
{
    fprintf(stderr, "Set aspect\n");
    if (m_aspect == aspect)
        return;
    m_aspect = aspect;
    if (m_state != STOPPED)
    {
        switch (aspect)
        {
            case ASPECT_DEFAULT: setProperty("video-aspect", 0); break;
            case ASPECT_4_3: setProperty("video-aspect", 4.0 / 3.0); break;
            case ASPECT_16_9: setProperty("video-aspect", 16.0 / 9.0); break;
            case ASPECT_16_10: setProperty("video-aspect", 16.0 / 10.0); break;
            case ASPECT_185_100: setProperty("video-aspect", 1.85); break;
            case ASPECT_235_100: setProperty("video-aspect", 2.35); break;
            default: break;
        }
    }
    emit aspectChanged();
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


// Handle mpv event
static void postEvent(void *ptr)
{
    MpvObject *obj = (MpvObject*) ptr;
    QMetaObject::invokeMethod(obj, "onMpvEvent", Qt::QueuedConnection);
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
            m_aspect = ASPECT_DEFAULT;
            emit aspectChanged();
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
            
            else if (propName == "track-list") // read tracks info
            {
                m_subtitles.clear();
                mpv_node* node = static_cast<mpv_node*>(prop->data);
                QVariantList trackList = mpv::qt::node_to_variant(node).toList();
                foreach (QVariant i, trackList)
                {
                    QVariantHash item = i.toHash();
                    if (item["type"].toString() == "sub")  // subtitle
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
                }
                emit subtitlesChanged();
            }

            else if (propName == "sid") // subtitle id
            {
                int sid = *(int64_t *) prop->data;
                if (m_sid != sid)
                {
                    m_sid = sid;
                    emit sidChanged();
                }
            }
            /*
            else if (propName == "track-list") // read tracks info
            {
                audioTracksList.clear();
                subtitleList.clear();
                mpv_node *node = (mpv_node *) prop->data;
                for (int i = 0; i < node->u.list->num; i++)
                {
                    mpv_node_list *item = node->u.list->values[i].u.list;
                    QByteArray type;
                    int id = 0;
                    QString title;
                    for (int n = 0; n < item->num; n++)
                    {
                        if (!strcmp(item->keys[n], "type"))
                            type = item->values[n].u.string;
                        else if (!strcmp(item->keys[n], "id"))
                            id = item->values[n].u.int64;
                        else if (!strcmp(item->keys[n], "title"))
                            title = QString::fromUtf8(item->values[n].u.string);
                    }
                    // subtitles
                    if (type == "sub")
                    {
                        if (subtitleList.size() <= id)
                        {
                            for (int j = subtitleList.size(); j < id; j++)
                                subtitleList.append('#' + QString::number(j));
                            subtitleList.append(title.isEmpty() ? '#' + QString::number(id) : title);
                        }
                        else
                            subtitleList[id] = title.isEmpty() ? '#' + QString::number(id) : title;
                    }
                    // audio tracks
                    if (type == "audio")
                    {
                        if (audioTracksList.size() <= id)
                        {
                            for (int j = audioTracksList.size(); j < id; j++)
                                audioTracksList.append('#' + QString::number(j));
                            audioTracksList.append(title.isEmpty() ? '#' + QString::number(id) : title);
                        }
                        else
                            audioTracksList[id] = title.isEmpty() ? '#' + QString::number(id) : title;
                    }
                }
            }*/
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
    return new MpvRenderer(this);
}


