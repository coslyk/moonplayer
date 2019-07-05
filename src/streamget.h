#ifndef STREAMGET_H
#define STREAMGET_H

#include "downloaderitem.h"
class QProcess;

class StreamGet : public DownloaderItem
{
    Q_OBJECT
public:
    StreamGet(const QUrl &url, const QString &filename, QObject *parent = NULL);
    ~StreamGet();
    void pause();
    void start();
    void stop();

private:
    QProcess *process;
    QStringList args;

private slots:
    void readOutput(void);
    void onProcFinished(int code);
};

#endif // STREAMGET_H
