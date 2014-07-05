#ifndef HTTPGET_H
#define HTTPGET_H

#include <QObject>
#include <QUrl>
class QString;
class QFile;
class QNetworkReply;
class QNetworkAccessManager;

class HttpGet : public QObject
{
    Q_OBJECT
public:
    static void setProxy(const QString &proxy, int port);
    explicit HttpGet(const QUrl &url, const QString &filename, QObject *parent = 0);
    inline QString &getFileName(void) {return name;}
    void pause(void);
    void start(void);
    void stop(void);

signals:
    void finished(HttpGet *self, bool error);
    void progressChanged(HttpGet *self, int progress, bool isPercentage);
    void paused(HttpGet *self, int reason);

private:
    QFile *file;
    QString name;
    QUrl url;
    QNetworkReply *reply;
    int prev_progress;
    qint64 last_finished;
    bool is_paused;
    static QNetworkAccessManager *manager;
    
private slots:
    void onFinished(void);
    void onReadyRead(void);
    void onProgressChanged(qint64 received, qint64 total);
};

#endif // HTTPGET_H
