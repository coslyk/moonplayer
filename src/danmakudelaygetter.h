#ifndef DANMAKUDELAYGETTER_H
#define DANMAKUDELAYGETTER_H

#include <QObject>
#include <QStringList>
#include <mpv/client.h>

// Get the danmaku delay for videos which is cut into clips so that danmaku can patch all clips
// After finished, the getter object will delete itself

class DanmakuDelayGetter : public QObject
{
    Q_OBJECT
public:
    DanmakuDelayGetter(QStringList &names, QStringList &urls, const QString &danmakuUrl,
                       bool download, QObject *parent = 0);
    ~DanmakuDelayGetter();

protected:
    bool event(QEvent *e);

private slots:
    void start(void);

private:
    QStringList names;
    QStringList urls;
    QString danmakuUrl;
    double delay;
    bool download;
    mpv_handle *mpv;
};

#endif // DANMAKUDELAYGETTER_H
