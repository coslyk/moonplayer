#include "mpvObject.h"
#include <mpv/qthelper.hpp>
#include <stdexcept>
#include <clocale>

#include <QDir>
#include <QOpenGLContext>
#include <QGuiApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QMetaType>

#include <QtGui/QOpenGLFramebufferObject>

#include "accessManager.h"
#include "danmakuLoader.h"
#include "playlistModel.h"


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
    MpvObject *m_obj;
    
public:
    MpvRenderer(MpvObject *obj) : m_obj(obj)
    {
    }

    // This function is called when a new FBO is needed.
    // This happens on the initial frame.
    QOpenGLFramebufferObject * createFramebufferObject(const QSize & size)
    {
        Q_ASSERT(QGuiApplication::platformNativeInterface() != nullptr);
        Q_ASSERT(m_obj != nullptr);

        // init mpv_gl
        if (!m_obj->m_mpv.renderer_initialized())
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
                params[2].type = MPV_RENDER_PARAM_X11_DISPLAY;
                params[2].data = QX11Info::display();
            } else {    // Wayland
                params[2].type = MPV_RENDER_PARAM_WL_DISPLAY;
                params[2].data = QGuiApplication::platformNativeInterface()->nativeResourceForWindow(QByteArrayLiteral("display"), NULL);
            }
#endif

            if (m_obj->m_mpv.renderer_initialize(params) < 0)
                throw std::runtime_error("failed to initialize mpv GL context");
            
            m_obj->m_mpv.set_render_callback([](void *ctx) {
                MpvObject *obj = reinterpret_cast<MpvObject*>(ctx);
                QMetaObject::invokeMethod(obj, "update", Qt::QueuedConnection);
            }, m_obj);
        }
        return QQuickFramebufferObject::Renderer::createFramebufferObject(size);
    }

    void render()
    {
        Q_ASSERT(m_obj != nullptr);
        Q_ASSERT(m_obj->window() != nullptr);

        m_obj->window()->resetOpenGLState();

        QOpenGLFramebufferObject *fbo = framebufferObject();
        Q_ASSERT(fbo != nullptr);

        mpv_opengl_fbo mpfbo {
            static_cast<int>(fbo->handle()),
            fbo->width(),
            fbo->height(),
            0
        };
        int flip_y = 0;

        mpv_render_param params[] = {
            {MPV_RENDER_PARAM_OPENGL_FBO, &mpfbo},
            {MPV_RENDER_PARAM_FLIP_Y, &flip_y},
            {MPV_RENDER_PARAM_INVALID, nullptr}
        };
        m_obj->m_mpv.render(params);

        m_obj->window()->resetOpenGLState();
    }
};


MpvObject* MpvObject::s_instance = nullptr;

MpvObject::MpvObject(QQuickItem * parent) : QQuickFramebufferObject(parent)
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = this;
    
    m_time = m_duration = 0;
    m_volume = 100;
    
    // Access settings
    QSettings settings;

    // set mpv options
    m_mpv.set_option_string("pause", "no");
    m_mpv.set_option_string("vo", "libmpv");           // Force to use libmpv
    m_mpv.set_option_string("softvol", "yes");         // mpv handles the volume
    m_mpv.set_option_string("ytdl", "no");             // We handle video url parsing
    m_mpv.set_option_string("screenshot-directory", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation).toUtf8().constData());
    m_mpv.set_option_string("reset-on-next-file", "speed,video-aspect,af,sub-visibility,audio-delay,pause");
    
    m_mpv.observe_property<int64_t>("duration");
    m_mpv.observe_property<int64_t>("playback-time");
    m_mpv.observe_property<int>("paused-for-cache");
    m_mpv.observe_property<int>("core-idle");
    m_mpv.observe_property<int>("pause");
    m_mpv.observe_property<mpv_node*>("track-list");
    m_mpv.request_log_messages("warn");
    
    
    // Configure hardware decoding
    bool hwdecCopy = settings.value(QStringLiteral("video/hwdec_copy_mode"), false).toBool();
    
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    Hwdec hwdec = (Hwdec) settings.value(QStringLiteral("video/hwdec"), 0).toInt();
    switch (hwdec)
    {
        case AUTO:
            m_mpv.set_option_string("gpu-hwdec-interop", "auto");
            m_mpv.set_option_string("hwdec", hwdecCopy ? "auto-copy" : "auto");
            break;
        case VAAPI:
            m_mpv.set_option_string("gpu-hwdec-interop", "vaapi-egl");
            m_mpv.set_option_string("hwdec", hwdecCopy ? "vaapi-copy" : "vaapi");
            break;
        case VDPAU:
            m_mpv.set_option_string("gpu-hwdec-interop", "vdpau-glx");
            m_mpv.set_option_string("hwdec", hwdecCopy ? "vdpau-copy" : "vdpau");
            break;
        default: break;
    }

#elif defined(Q_OS_MAC)
    m_mpv.set_option_string("gpu-hwdec-interop", "videotoolbox");
    m_mpv.set_option_string("hwdec", hwdecCopy ? "videotoolbox-copy" : "videotoolbox");
    
#elif defined(Q_OS_WIN)
    m_mpv.set_option_string("gpu-context", "angle");
    if (QSysInfo::productVersion() == QStringLiteral("8.1") || QSysInfo::productVersion() == QStringLiteral("10"))
        m_mpv.set_option_string("hwdec", hwdecCopy ? "d3d11va-copy" : "d3d11va");
    else
        m_mpv.set_option_string("hwdec", hwdecCopy ? "dxva2-copy" : "dxva2");
#endif
    
    if (m_mpv.initialize() < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Set update callback
    m_mpv.set_wakeup_callback([](void *ctx) {
        MpvObject *obj = reinterpret_cast<MpvObject *>(ctx);
        QMetaObject::invokeMethod(obj, "onMpvEvent", Qt::QueuedConnection);
    }, this);
}


// Open file
void MpvObject::open(const QUrl& fileUrl, const QUrl& danmakuUrl, const QUrl& audioTrack)
{
    Q_ASSERT(NetworkAccessManager::instance() != nullptr);
    
    // set network parameters
    if (!fileUrl.isLocalFile())
    {
        // set referer
        m_mpv.set_option_string("referrer", NetworkAccessManager::instance()->refererOf(fileUrl));

        // set user-agent
        m_mpv.set_option_string("user-agent", NetworkAccessManager::instance()->userAgentOf(fileUrl));

        /* Some websites does not allow "Range" option in http request header.
         * To hack these websites, we force ffmpeg/libav to set the stream unseekable.
         * Then we make the video seekable again by enabling seeking in cache.
         */
        if (NetworkAccessManager::instance()->urlIsUnseekable(fileUrl))
        {
            m_mpv.set_option_string("stream-lavf-o", "seekable=0");
            m_mpv.set_option_string("force-seekable", "yes");
        }
        else
        {
            m_mpv.set_option_string("stream-lavf-o", "");
            m_mpv.set_option_string("force-seekable", "no");
        }
    }
    else
    {
        m_mpv.set_option_string("stream-lavf-o", "");
        m_mpv.set_option_string("force-seekable", "no");
    }
    
    QByteArray fileuri_str = (fileUrl.isLocalFile() ? fileUrl.toLocalFile() : fileUrl.toString()).toUtf8();
    const char *args[] = {"loadfile", fileuri_str.constData(), nullptr};
    m_mpv.command_async(args);
    m_danmakuUrl = danmakuUrl;
    m_audioToBeAdded = audioTrack;
}


// Play, Pause, Stop & Get state
void MpvObject::play()
{
    if (m_state == VIDEO_PAUSED)
    {
        m_mpv.set_property_async("pause", 0);
    }
}

void MpvObject::pause()
{
    if (m_state == VIDEO_PLAYING)
    {
        m_mpv.set_property_async("pause", 1);
    }
}

void MpvObject::stop()
{
    if (m_state != STOPPED)
    {
        const char *args[] = {"stop", nullptr};
        m_mpv.command_async(args);
    }
}

// Seek
void MpvObject::seek ( qint64 time, bool absolute )
{
    if (m_state != STOPPED && time != m_time)
    {
        QByteArray time_str = QByteArray::number(time);
        const char *args[] = {"seek", time_str.constData(), (absolute ? "absolute" : "relative"), nullptr};
        m_mpv.command_async(args);
    }
}


// Set volume
void MpvObject::setVolume(int volume)
{
    if (m_volume == volume)
        return;
    
    double vol = volume;
    m_mpv.set_property_async("volume", vol);
    showText(QByteArrayLiteral("Volume: ") + QByteArray::number(volume));
    
    m_volume = volume;
    emit volumeChanged();
}

// Set subtitle visibility
void MpvObject::setSubVisible(bool subVisible)
{
    if (m_subVisible == subVisible)
        return;
    m_subVisible = subVisible;
    m_mpv.set_property_async("sub-visibility", static_cast<int>(m_subVisible));
    emit subVisibleChanged();
}

// Set speed
void MpvObject::setSpeed(double speed)
{
    if (m_speed - speed < 0.1 && speed - m_speed < 0.1)
        return;
    m_speed = speed;
    m_mpv.set_property_async("speed", m_speed);
    showText(QByteArrayLiteral("Speed: ") + QByteArray::number(m_speed));
    emit speedChanged();
}


// Add audio track
void MpvObject::addAudioTrack(const QUrl& url)
{
    if (m_state == STOPPED)
        return;
    QByteArray uri_str = (url.isLocalFile() ? url.toLocalFile() : url.toString()).toUtf8();
    const char *args[] = {"audio-add", uri_str.constData(), "select", nullptr};
    m_mpv.command_async(args);
}

// Add subtitle
void MpvObject::addSubtitle(const QUrl& url)
{
    if (m_state == STOPPED)
        return;
    QByteArray uri_str = (url.isLocalFile() ? url.toLocalFile() : url.toString()).toUtf8();
    const char *args[] = {"sub-add", uri_str.constData(), "select", nullptr};
    m_mpv.command_async(args);
}

// Add danmaku
void MpvObject::addDanmaku(const Danmaku2ASS::AssBuilder::Ptr& danmakuAss)
{
    m_danmakuAss = danmakuAss;
    if (danmakuAss != nullptr)
    {
        QString outputFile = QDir::temp().filePath(QStringLiteral("moonplayer_danmaku.ass"));
        danmakuAss->setDisallowMode(m_danmakuDisallowMode);
        danmakuAss->setBlockWords(m_blockWords);
        danmakuAss->setReservedArea(m_reservedArea);
        danmakuAss->exportAssToFile(outputFile.toStdString());
        addSubtitle(QUrl::fromLocalFile(outputFile));
    }
}

// Reload danmaku
void MpvObject::reloadDanmaku(bool top, bool bottom, bool scrolling, double reservedArea, const QStringList& blockWords)
{
    // Reserved area
    m_reservedArea = reservedArea;

    // Reset disallow mode
    m_danmakuDisallowMode = 0;
    if (!top)
    {
        m_danmakuDisallowMode |= Danmaku2ASS::DISALLOW_TOP;
    }
    if (!bottom)
    {
        m_danmakuDisallowMode |= Danmaku2ASS::DISALLOW_BOTTOM;
    }
    if (!scrolling)
    {
        m_danmakuDisallowMode |= Danmaku2ASS::DISALLOW_SCROLL;
    }

    // Convert block words list
    m_blockWords.clear();
    for (const auto& word : blockWords)
    {
        m_blockWords.push_back(word.toStdString());
    }

    // Reload danmaku
    if (m_danmakuAss != nullptr && m_state != STOPPED)
    {
        m_danmakuAss->setDisallowMode(m_danmakuDisallowMode);
        m_danmakuAss->setBlockWords(m_blockWords);
        m_danmakuAss->setReservedArea(m_reservedArea);

        // Export ass file
        QString outputFile = QDir::temp().filePath(QStringLiteral("moonplayer_danmaku.ass"));
        m_danmakuAss->exportAssToFile(outputFile.toStdString());

        // Reload ass file
        const char *args[] = {"sub-reload", "1", nullptr};
        m_mpv.command_async(args);
    }
}

// Take screenshot
void MpvObject::screenshot()
{
    if (m_state == STOPPED)
        return;
    const char *args[] = {"osd-msg", "screenshot", nullptr};
    m_mpv.command_async(args);
}


void MpvObject::onMpvEvent()
{
    while (true)
    {
        mpv_event *event = m_mpv.wait_event();
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
            m_state = VIDEO_PLAYING;
            emit stateChanged();
            break;

        case MPV_EVENT_END_FILE:
        {
            mpv_event_end_file *ef = static_cast<mpv_event_end_file*>(event->data);
            handleMpvError(ef->error);
            m_endFileReason = static_cast<mpv_end_file_reason>(ef->reason);
            break;
        }
        
        case MPV_EVENT_IDLE:
        {
            if (m_endFileReason == MPV_END_FILE_REASON_EOF)
            {
                Q_ASSERT(PlaylistModel::instance() != nullptr);
                PlaylistModel::instance()->playNextItem();
            }
            else
            {
                m_state = STOPPED;
                emit stateChanged();
            }
            break;
        }

        case MPV_EVENT_VIDEO_RECONFIG:
            if (!m_audioToBeAdded.isEmpty())
            {
                addAudioTrack(m_audioToBeAdded);
                m_audioToBeAdded = QUrl();
            }
            m_videoWidth = m_mpv.get_property<int64_t>("dwidth");
            m_videoHeight = m_mpv.get_property<int64_t>("dheight");
            emit videoSizeChanged();

            // Load danmaku
            if (!m_danmakuUrl.isEmpty())
            {
                Q_ASSERT(DanmakuLoader::instance() != nullptr);
                DanmakuLoader::instance()->start(m_danmakuUrl, m_videoWidth, m_videoHeight);
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
                qint64 newTime = *(qint64*) prop->data;
                if (newTime != m_time)
                {
                    m_time = newTime;
                    emit timeChanged();
                }
            }

            else if (propName == "duration")
            {
                m_duration = *(qint64*) prop->data;
                emit durationChanged();
            }

            else if (propName == "pause")
            {
                int pause = *(int*) prop->data;
                if (pause && m_state == VIDEO_PLAYING)
                {
                    m_state = VIDEO_PAUSED;
                }
                else if (m_state == VIDEO_PAUSED)
                {
                    m_state = VIDEO_PLAYING;
                }
                emit stateChanged();
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
                    if (item[QStringLiteral("type")].toString() == QStringLiteral("sub"))  // Subtitles
                    {
                        int id = item[QStringLiteral("id")].toInt();
                        QString title = item[QStringLiteral("title")].toString();
                        if (m_subtitles.count() <= id)
                        {
                            for (int j = m_subtitles.count(); j < id; j++)
                                m_subtitles.append(QLatin1Char('#') + QString::number(j));
                            m_subtitles.append(title.isEmpty() ? QLatin1Char('#') + QString::number(id) : title);
                        }
                        else
                        {
                            m_subtitles[id] = title.isEmpty() ? QLatin1Char('#') + QString::number(id) : title;
                        }
                    }

                    else if (item[QStringLiteral("type")].toString() == QStringLiteral("audio"))  // Audio tracks
                    {
                        int id = item[QStringLiteral("id")].toInt();
                        QString title = item[QStringLiteral("title")].toString();
                        if (m_audioTracks.count() <= id)
                        {
                            for (int j = m_audioTracks.count(); j < id; j++)
                                m_audioTracks.append(QLatin1Char('#') + QString::number(j));
                            m_audioTracks.append(title.isEmpty() ? QLatin1Char('#') + QString::number(id) : title);
                        }
                        else
                        {
                            m_audioTracks[id] = title.isEmpty() ? QLatin1Char('#') + QString::number(id) : title;
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


// setProperty() exposed to QML
void MpvObject::setProperty(const QString &name, const QVariant &value)
{
    switch ((int) value.type())
    {
        case (int) QMetaType::Bool:
        {
            int v = value.toInt();
            m_mpv.set_property_async (name.toLatin1().constData(), v);
            break;
        }
        case (int) QMetaType::Int:
        case (int) QMetaType::Long:
        case (int) QMetaType::LongLong:
        {
            int64_t v = value.toLongLong();
            m_mpv.set_property_async(name.toLatin1().constData(), v);
            break;
        }
        case (int) QMetaType::Float:
        case (int) QMetaType::Double:
        {
            double v = value.toDouble();
            m_mpv.set_property_async(name.toLatin1().constData(), v);
            break;
        }
        case (int) QMetaType::QByteArray:
        {
            QByteArray v = value.toByteArray();
            m_mpv.set_property_async(name.toLatin1().constData(), v.constData());
            break;
        }
        case (int) QMetaType::QString:
        {
            QByteArray v = value.toString().toUtf8();
            m_mpv.set_property_async(name.toLatin1().constData(), v.constData());
            break;
        }
    }
}

void MpvObject::handleMpvError (int code)
{
    if (code < 0)
    {
        qDebug() << "MPV Error: " << mpv_error_string(code);
    }
}

void MpvObject::showText(const QByteArray& text)
{
    const char *args[] = {"show-text", text.constData(), nullptr};
    m_mpv.command_async(args);
}



QQuickFramebufferObject::Renderer *MpvObject::createRenderer() const
{
    Q_ASSERT(window() != nullptr);

    window()->setPersistentOpenGLContext(true);
    window()->setPersistentSceneGraph(true);
    return new MpvRenderer(const_cast<MpvObject*>(this));
}


