#ifndef STREAMGET_H
#define STREAMGET_H

#include "downloaderitem.h"
class QProcess;
class QTimer;

class StreamGet : public DownloaderItem
{
    Q_OBJECT
public:
    StreamGet(const QUrl &url, const QString &filename, QObject *parent = nullptr);
    ~StreamGet();
    void pause();
    void start();
    void stop();

private:
    QProcess *process;
    QTimer *timer;
    QStringList args;
    int duration;

private slots:
    void readOutput(void);
    void onProcFinished(int code);
};

#endif // STREAMGET_H
