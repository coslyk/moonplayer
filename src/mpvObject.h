#ifndef MPVRENDERER_H_
#define MPVRENDERER_H_

#include <QtQuick/QQuickFramebufferObject>
#include <QQuickWindow>

#include <mpv/client.h>
#include <mpv/opengl_cb.h>
#include <mpv/qthelper.hpp>

class MpvRenderer;

class MpvObject : public QQuickFramebufferObject
{
    Q_OBJECT
    
    Q_PROPERTY(State state            READ state                            NOTIFY stateChanged)
    Q_PROPERTY(Aspect aspect          READ aspect      WRITE setAspect      NOTIFY aspectChanged)
    Q_PROPERTY(qint64 duration        READ duration                         NOTIFY durationChanged)
    Q_PROPERTY(qint64 time            READ time                             NOTIFY timeChanged)
    Q_PROPERTY(int sid                READ sid         WRITE setSid         NOTIFY sidChanged)
    Q_PROPERTY(int volume             READ volume      WRITE setVolume      NOTIFY volumeChanged)
    Q_PROPERTY(QSize videoSize        READ videoSize                        NOTIFY videoSizeChanged)
    Q_PROPERTY(bool subVisible        READ subVisible  WRITE setSubVisible  NOTIFY subVisibleChanged)
    Q_PROPERTY(double speed           READ speed       WRITE setSpeed       NOTIFY speedChanged)
    Q_PROPERTY(QStringList subtitles  READ subtitles                        NOTIFY subtitlesChanged)
    
    friend class MpvRenderer;

public:
    enum State {STOPPED, VIDEO_PLAYING, VIDEO_PAUSED, TV_PLAYING};
    enum Aspect {ASPECT_DEFAULT, ASPECT_4_3, ASPECT_16_9, ASPECT_16_10, ASPECT_185_100, ASPECT_235_100};
    enum Hwdec {AUTO, VAAPI, VDPAU};
    Q_ENUM(State)
    Q_ENUM(Aspect)
    
    inline static MpvObject* instance() { return s_instance; }
    
    MpvObject(QQuickItem * parent = nullptr);
    virtual ~MpvObject();
    virtual Renderer *createRenderer() const;
    
    // Access properties
    inline QSize videoSize() { return QSize(m_videoWidth, m_videoHeight) / window()->effectiveDevicePixelRatio(); }
    inline State state() { return m_state; }
    inline Aspect aspect() { return m_aspect; }
    inline qint64 duration() { return m_duration; }
    inline qint64 time() { return m_time; }
    inline double speed() { return m_speed; }
    inline bool subVisible() { return m_subVisible; }
    inline int volume() { return m_volume; }
    inline int sid() { return m_sid; }
    inline QStringList subtitles() { return m_subtitles; }

    void setAspect(Aspect aspect);
    void setVolume(int volume);
    void setSubVisible(bool subVisible);
    void setSpeed(double speed);
    void setSid(int sid);
    
    
public slots:
    void open(const QUrl& fileUrl, const QUrl& danmakuUrl = QUrl(), const QUrl& audioTrackUrl = QUrl());
    void play(void);
    void pause(void);
    void stop(void);
    void seek(qint64 offset, bool absolute = true);
    void screenshot(void);
    void addAudioTrack(const QUrl& url);
    void addSubtitle(const QUrl& url);
    void command(const QStringList& params);
    void setProperty(const QString& name, const QVariant& value);
    void showText(const QString& text);
    
signals:
    void onUpdate(void);
    void stopped(bool stoppedByUser);
    void aspectChanged(void);
    void sidChanged(void);
    void stateChanged(void);
    void speedChanged(void);
    void subtitlesChanged(void);
    void subVisibleChanged(void);
    void durationChanged(void);
    void timeChanged(void);
    void volumeChanged(void);
    void videoSizeChanged(void);

private slots:
    void doUpdate();
    
private:
    static void on_update(void *ctx);
    Q_INVOKABLE void onMpvEvent(void);
    void handleMpvError(int code);
    
    mpv::qt::Handle mpv;
    mpv_opengl_cb_context *mpv_gl;
    bool no_emit_stopped;
    bool emit_stopped_when_idle;
    
    State m_state;
    Aspect m_aspect;
    QUrl m_danmakuUrl;
    QUrl m_audioToBeAdded;
    qint64 m_time;
    qint64 m_duration;
    bool m_stopByUser;
    bool m_subVisible;
    int m_sid;
    int m_volume;
    int m_videoWidth;
    int m_videoHeight;
    double m_speed;
    QStringList m_subtitles;
    
    static MpvObject* s_instance;
};

#endif

