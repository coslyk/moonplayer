#ifndef HTTPGET_H
#define HTTPGET_H

#include <QUrl>
#include "downloaderitem.h"
class QString;
class QFile;
class QNetworkReply;

class HttpGet : public DownloaderItem
{
    Q_OBJECT
public:
    explicit HttpGet(const QUrl &url, const QString &filename, QObject *parent = NULL);
    void pause(void);
    void start(void);
    void stop(void);

private:
    QFile *file;
    QString name;
    QUrl url;
    QNetworkReply *reply;
    int prev_progress;
    qint64 last_finished;
    bool is_paused;
    
private slots:
    void onFinished(void);
    void onReadyRead(void);
    void onProgressChanged(qint64 received, qint64 total);
};

#endif // HTTPGET_H
