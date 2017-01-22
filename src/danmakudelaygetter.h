#ifndef DANMAKUDELAYGETTER_H
#define DANMAKUDELAYGETTER_H

#include <QObject>
#include <QStringList>
class QProcess;

#ifdef Q_OS_MAC
#include <vlc/vlc.h>
#endif

// Get the danmaku delay for videos which is cut into clips so that danmaku can patch all clips
// After finished, the getter object will delete itself

class DanmakuDelayGetter : public QObject
{
    Q_OBJECT
public:
    DanmakuDelayGetter(QStringList &names, QStringList &urls, const QString &danmakuUrl,
                       bool download, QObject *parent = 0);
private slots:
    void start(void);

private:
    QStringList names;
    QStringList urls;
    QString danmakuUrl;
    double delay;
    bool download;

#ifdef Q_OS_MAC
public:
    static inline void setVlcInstance(libvlc_instance_t *i) {vlcInstance = i;}
signals:
    void nextClip(void);
    void failed(void);
private:
    libvlc_media_t *vlcMedia;
    static libvlc_instance_t *vlcInstance;
    static void onFinished(const libvlc_event_t *, DanmakuDelayGetter *d);
private slots:
    void onFailed(void);

#else
private:
    static bool dummy_mode;
    QProcess *process;
private slots:
    void onFinished(void);
#endif
};

#endif // DANMAKUDELAYGETTER_H
