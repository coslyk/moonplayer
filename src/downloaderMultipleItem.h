#ifndef DOWNLOADERMULTIPLEITEM_H
#define DOWNLOADERMULTIPLEITEM_H

#include "downloaderAbstractItem.h"
#include <QDir>

class DownloaderSingleItem;
class QProcess;

class DownloaderMultipleItem : public DownloaderAbstractItem
{
    Q_OBJECT

public:
    DownloaderMultipleItem(const QString &filepath, const QList<QUrl>& urls, const QUrl& danmakuUrl = QUrl(), bool isDash = false, QObject *parent = nullptr);
    virtual void pause(void);
    virtual void start(void);
    virtual void stop(bool continueWaiting = true);
    
private:
    QList<DownloaderSingleItem*> m_items;
    QDir m_tempDir;
    int m_finished;
    int m_total;
    bool m_isDash;
    QProcess* m_process;
    
    void concatVideos(void);
    
private slots:
    void onItemStateChanged(DownloaderSingleItem* item);
    void onItemProgressChanged(void);
    void onConcatFinished(int status);
};

#endif // DOWNLOADERMULTIPLEITEM_H
