#ifndef FILEDOWNLOADER_H
#define FILEDOWNLOADER_H

#include <QFile>
#include <QUrl>

class QNetworkReply;

class FileDownloader : public QObject
{
    Q_OBJECT

signals:
    void started();
    void paused();
    void stopped();
    void finished();
    void progressChanged(int progress);
    
public:
    FileDownloader(const QString &filepath, const QUrl &url, QObject *parent = nullptr);
    virtual ~FileDownloader();
    void pause(void);
    void start(void);
    void stop(void);
    inline int progress() const { return m_progress; }

private:
    QNetworkReply* m_reply;
    QFile m_file;
    QUrl m_url;
    qint64 m_lastPos;
    int m_progress;
    
private slots:
    void onFinished(void);
    void onReadyRead(void);
    void onDownloadProgressChanged(qint64 received, qint64 total);
};

#endif // FILEDOWNLOADER_H
