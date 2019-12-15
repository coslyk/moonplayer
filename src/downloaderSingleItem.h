#ifndef DOWNLOADERSINGLEITEM_H
#define DOWNLOADERSINGLEITEM_H

#include "downloaderAbstractItem.h"
#include <QFile>
#include <QUrl>

class QFile;
class QNetworkReply;

class DownloaderSingleItem : public DownloaderAbstractItem
{
    Q_OBJECT
    
public:
    DownloaderSingleItem(const QString &filepath, const QUrl &url, QObject *parent = nullptr);
    virtual void pause(void);
    virtual void start(void);
    virtual void stop(bool continueWaiting = true);

private:
    static QList<DownloaderSingleItem*> s_waitingItems;
    static int s_numOfRunning;
    
    QNetworkReply* m_reply;
    QFile m_file;
    QUrl m_url;
    qint64 m_lastPos;
    
    static void continueWaitingTasks(void);
    
private slots:
    void onFinished(void);
    void onReadyRead(void);
    void onDownloadProgressChanged(qint64 received, qint64 total);
};

#endif // DOWNLOADERSINGLEITEM_H
