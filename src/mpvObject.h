#ifndef MPVRENDERER_H_
#define MPVRENDERER_H_

#include <QtQuick/QQuickFramebufferObject>
#include <QQuickWindow>

#include "mpv.hpp"
#include <mpv/client.h>
#include <mpv/render_gl.h>

#include <Danmaku2ASS/AssBuilder.h>

class MpvRenderer;

class MpvObject : public QQuickFramebufferObject
{
    Q_OBJECT
    
    Q_PROPERTY(State state              READ state                            NOTIFY stateChanged)
    Q_PROPERTY(qint64 duration          READ duration                         NOTIFY durationChanged)
    Q_PROPERTY(qint64 time              READ time                             NOTIFY timeChanged)
    Q_PROPERTY(int volume               READ volume      WRITE setVolume      NOTIFY volumeChanged)
    Q_PROPERTY(QSize videoSize          READ videoSize                        NOTIFY videoSizeChanged)
    Q_PROPERTY(bool subVisible          READ subVisible  WRITE setSubVisible  NOTIFY subVisibleChanged)
    Q_PROPERTY(double speed             READ speed       WRITE setSpeed       NOTIFY speedChanged)
    Q_PROPERTY(QStringList audioTracks  READ audioTracks                      NOTIFY audioTracksChanged)
    Q_PROPERTY(QStringList subtitles    READ subtitles                        NOTIFY subtitlesChanged)
    
    friend class MpvRenderer;

public:
    enum State {STOPPED, VIDEO_PLAYING, VIDEO_PAUSED, TV_PLAYING};
    enum Hwdec {AUTO, VAAPI, VDPAU};
    Q_ENUM(State)
    
    inline static MpvObject* instance() { return s_instance; }
    
    MpvObject(QQuickItem * parent = nullptr);
    virtual ~MpvObject();
    virtual Renderer *createRenderer() const;
    
    // Access properties
    inline QSize videoSize() { return QSize(m_videoWidth, m_videoHeight) / window()->effectiveDevicePixelRatio(); }
    inline State state() { return m_state; }
    inline qint64 duration() { return m_duration; }
    inline qint64 time() { return m_time; }
    inline double speed() { return m_speed; }
    inline bool subVisible() { return m_subVisible; }
    inline int volume() { return m_volume; }
    inline QStringList audioTracks() { return m_audioTracks; }
    inline QStringList subtitles() { return m_subtitles; }

    void setVolume(int volume);
    void setSubVisible(bool subVisible);
    void setSpeed(double speed);

public slots:
    void open(const QUrl& fileUrl, const QUrl& danmakuUrl = QUrl(), const QUrl& audioTrackUrl = QUrl());
    void play(void);
    void pause(void);
    void stop(void);
    void seek(qint64 offset, bool absolute = true);
    void screenshot(void);
    void addAudioTrack(const QUrl& url);
    void addDanmaku(const Danmaku2ASS::AssBuilder::Ptr& danmakuAss);
    void addSubtitle(const QUrl& url);
    void reloadDanmaku(bool top, bool bottom, bool scrolling, double reservedArea, const QStringList& blockWords);
    void setProperty(const QString& name, const QVariant& value);
    void showText(const QByteArray &text);

signals:
    void stopped(bool stoppedByUser);
    void audioTracksChanged(void);
    void stateChanged(void);
    void speedChanged(void);
    void subtitlesChanged(void);
    void subVisibleChanged(void);
    void durationChanged(void);
    void timeChanged(void);
    void volumeChanged(void);
    void videoSizeChanged(void);
    
private:
    static void on_update(void *ctx);
    Q_INVOKABLE void onMpvEvent(void);
    void handleMpvError(int code);
    
    Mpv::Handle m_mpv;
    mpv_render_context *mpv_gl;
    bool no_emit_stopped;
    bool emit_stopped_when_idle;
    
    State m_state;
    bool m_stopByUser;
    bool m_subVisible;
    int m_volume;
    int m_danmakuDisallowMode;
    int64_t m_time;
    int64_t m_duration;
    int64_t m_videoWidth;
    int64_t m_videoHeight;
    double m_speed;
    double m_reservedArea;
    QUrl m_danmakuUrl;
    QUrl m_audioToBeAdded;
    QStringList m_audioTracks;
    QStringList m_subtitles;
    Danmaku2ASS::AssBuilder::Ptr m_danmakuAss;
    std::vector<std::string> m_blockWords;
    
    static MpvObject* s_instance;
};

#endif

