/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#include "mpvObject.h"
#include <stdexcept>
#include <clocale>

#include <QDir>
#include <QOpenGLContext>
#include <QSettings>
#include <QStandardPaths>
#include <QMetaType>

#include <QtGui/QOpenGLFramebufferObject>

#include "accessManager.h"
#include "danmakuLoader.h"
#include "playlistModel.h"
#include "platform/graphics.h"


/* MPV Renderer */
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
        Q_ASSERT(m_obj != nullptr);

        // init mpv_gl
        if (!m_obj->m_mpv.renderer_initialized())
        {
            mpv_opengl_init_params gl_init_params {
                [](void *, const char *name) -> void* {
                    QOpenGLContext *glctx = QOpenGLContext::currentContext();
                    return glctx ? reinterpret_cast<void *>(glctx->getProcAddress(QByteArray(name))) : nullptr;
                },
                nullptr, nullptr
            };

            mpv_render_param params[] {
                { MPV_RENDER_PARAM_API_TYPE,           const_cast<char *>(MPV_RENDER_API_TYPE_OPENGL) },
                { MPV_RENDER_PARAM_OPENGL_INIT_PARAMS, &gl_init_params},
                { MPV_RENDER_PARAM_X11_DISPLAY,        Graphics::x11Display() },
                { MPV_RENDER_PARAM_WL_DISPLAY,         Graphics::waylandDisplay() },
                { MPV_RENDER_PARAM_INVALID, nullptr}
            };

            if (m_obj->m_mpv.renderer_initialize(params) < 0)
                throw std::runtime_error("failed to initialize mpv GL context");
            
            m_obj->m_mpv.set_render_callback([](void *ctx) {
                MpvObject *obj = static_cast<MpvObject*>(ctx);
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
    m_mpv.set_option("ytdl", false);           // We handle video url parsing
    m_mpv.set_option("pause", false);          // Always play when a new file is opened
    m_mpv.set_option("softvol", true);         // mpv handles the volume
    m_mpv.set_option("vo", "libmpv");          // Force to use libmpv
    m_mpv.set_option("screenshot-directory", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation).toUtf8().constData());
    m_mpv.set_option("reset-on-next-file", "speed,video-aspect,af,sub-visibility,audio-delay,pause");
    
    m_mpv.observe_property("duration");
    m_mpv.observe_property("playback-time");
    m_mpv.observe_property("paused-for-cache");
    m_mpv.observe_property("core-idle");
    m_mpv.observe_property("pause");
    m_mpv.observe_property("track-list");
    m_mpv.request_log_messages("warn");

    // Configure cache
    if (settings.value(QStringLiteral("network/limit_cache"), false).toBool())
    {
        int64_t forwardBytes = settings.value(QStringLiteral("network/forward_cache")).toLongLong() << 20;
        int64_t backwardBytes = settings.value(QStringLiteral("network/backward_cache")).toLongLong() << 20;
        m_mpv.set_option("demuxer-max-bytes", forwardBytes);
        m_mpv.set_option("demuxer-max-back-bytes", backwardBytes);
    }
    
    // Configure hardware decoding
    bool hwdecCopy = settings.value(QStringLiteral("video/hwdec_copy_mode"), false).toBool();
    
#if defined(Q_OS_UNIX) && !defined(Q_OS_MAC)
    Hwdec hwdec = (Hwdec) settings.value(QStringLiteral("video/hwdec"), 0).toInt();
    switch (hwdec)
    {
        case AUTO:
            m_mpv.set_option("gpu-hwdec-interop", "auto");
            m_mpv.set_option("hwdec", hwdecCopy ? "auto-copy" : "auto");
            break;
        case VAAPI:
            m_mpv.set_option("gpu-hwdec-interop", "vaapi-egl");
            m_mpv.set_option("hwdec", hwdecCopy ? "vaapi-copy" : "vaapi");
            break;
        case VDPAU:
            m_mpv.set_option("gpu-hwdec-interop", "vdpau-glx");
            m_mpv.set_option("hwdec", hwdecCopy ? "vdpau-copy" : "vdpau");
            break;
        case NVDEC:
            m_mpv.set_option("hwdec", hwdecCopy ? "nvdec-copy" : "nvdec");
            break;
        default: break;
    }

#elif defined(Q_OS_MAC)
    m_mpv.set_option("gpu-hwdec-interop", "videotoolbox");
    m_mpv.set_option("hwdec", hwdecCopy ? "videotoolbox-copy" : "videotoolbox");
    
#elif defined(Q_OS_WIN)
    m_mpv.set_option("gpu-context", "angle");
    if (QSysInfo::productVersion() == QStringLiteral("8.1") || QSysInfo::productVersion() == QStringLiteral("10"))
        m_mpv.set_option("hwdec", hwdecCopy ? "d3d11va-copy" : "d3d11va");
    else
        m_mpv.set_option("hwdec", hwdecCopy ? "dxva2-copy" : "dxva2");
#endif
    
    if (m_mpv.initialize() < 0)
        throw std::runtime_error("could not initialize mpv context");

    // Set update callback
    m_mpv.set_wakeup_callback([](void *ctx) {
        MpvObject *obj = static_cast<MpvObject *>(ctx);
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
        m_mpv.set_option("referrer", NetworkAccessManager::instance()->refererOf(fileUrl).constData());

        // set user-agent
        m_mpv.set_option("user-agent", NetworkAccessManager::instance()->userAgentOf(fileUrl).constData());

        /* Some websites does not allow "Range" option in http request header.
         * To hack these websites, we force ffmpeg/libav to set the stream unseekable.
         * Then we make the video seekable again by enabling seeking in cache.
         */
        if (NetworkAccessManager::instance()->urlIsUnseekable(fileUrl))
        {
            m_mpv.set_option("stream-lavf-o", "seekable=0");
            m_mpv.set_option("force-seekable", true);
        }
        else
        {
            m_mpv.set_option("stream-lavf-o", "");
            m_mpv.set_option("force-seekable", false);
        }
    }
    else
    {
        m_mpv.set_option("stream-lavf-o", "");
        m_mpv.set_option("force-seekable", false);
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
        m_mpv.set_property_async("pause", false);
    }
}

void MpvObject::pause()
{
    if (m_state == VIDEO_PLAYING)
    {
        m_mpv.set_property_async("pause", true);
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

    m_volume = volume;
    m_mpv.set_property_async("volume", static_cast<double>(volume));
    showText(QByteArrayLiteral("Volume: ") + QByteArray::number(volume));
    emit volumeChanged();
}

// Set subtitle visibility
void MpvObject::setSubVisible(bool subVisible)
{
    if (m_subVisible == subVisible)
        return;
    m_subVisible = subVisible;
    m_mpv.set_property_async("sub-visibility", m_subVisible);
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
        const mpv_event *event = m_mpv.wait_event();
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
            Q_ASSERT(PlaylistModel::instance() != nullptr);
            if (m_endFileReason == MPV_END_FILE_REASON_EOF && PlaylistModel::instance()->hasNextItem())
            {
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
        {
            Mpv::Node width = m_mpv.get_property("dwidth");
            Mpv::Node height = m_mpv.get_property("dheight");

            if (width.type() != MPV_FORMAT_NONE)
            {
                m_videoWidth = width;
                m_videoHeight = height;
                emit videoSizeChanged();

                // Load audio track
                if (!m_audioToBeAdded.isEmpty())
                {
                    addAudioTrack(m_audioToBeAdded);
                    m_audioToBeAdded = QUrl();
                }

                // Load danmaku
                if (!m_danmakuUrl.isEmpty())
                {
                    Q_ASSERT(DanmakuLoader::instance() != nullptr);
                    DanmakuLoader::instance()->start(m_danmakuUrl, m_videoWidth, m_videoHeight);
                }
            }
            break;
        }

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
            {
                break;
            }

            const Mpv::Node &propValue = *static_cast<Mpv::Node*>(prop->data);
            if (propValue.type() == MPV_FORMAT_NONE)
            {
                break;
            }

            if (strcmp(prop->name, "playback-time") == 0)
            {
                int64_t newTime = static_cast<double>(propValue);  // It's double in mpv
                if (newTime != m_time)
                {
                    m_time = newTime;
                    emit timeChanged();
                }
            }

            else if (strcmp(prop->name, "duration") == 0)
            {
                m_duration = static_cast<double>(propValue);  // It's double in mpv
                emit durationChanged();
            }

            else if (strcmp(prop->name, "pause") == 0)
            {
                if (propValue && m_state == VIDEO_PLAYING)
                {
                    m_state = VIDEO_PAUSED;
                }
                else if (!propValue && m_state == VIDEO_PAUSED)
                {
                    m_state = VIDEO_PLAYING;
                }
                emit stateChanged();
            }

            else if (strcmp(prop->name,"paused-for-cache") == 0)
            {
                if (propValue && m_state != STOPPED)
                {
                    showText(QByteArrayLiteral("Network is slow..."));
                }
                else
                {
                    showText(QByteArrayLiteral(""));
                }
            }

            else if (strcmp(prop->name, "core-idle") == 0)
            {
                if (propValue && m_state == VIDEO_PLAYING)
                {
                    showText(QByteArrayLiteral("Buffering..."));
                }
                else
                {
                    showText(QByteArrayLiteral(""));
                }
            }
            
            else if (strcmp(prop->name, "track-list") == 0) // Read tracks info
            {
                m_subtitles.clear();
                m_audioTracks.clear();
                for (const auto& track : propValue)
                {
                    try
                    {
                        if (track["type"] == "sub") // Subtitles
                        {
                            int64_t id = track["id"];
                            QString title = QString::fromUtf8(track["title"]);
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

                        else if (track["type"] == "audio") // Audio tracks
                        {
                            int64_t id = track["id"];
                            QString title = QString::fromUtf8(track["title"]);
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
                    catch(const std::exception&)
                    {
                        continue;
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
            bool v = value.toBool();
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


