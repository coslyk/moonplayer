#ifndef DOWNLOADERMULTIPLEITEM_H
#define DOWNLOADERMULTIPLEITEM_H

#include "downloaderAbstractItem.h"
#include <QDir>
#include <atomic>

class QProcess;
class FileDownloader;

class DownloaderItem : public DownloaderAbstractItem
{
    Q_OBJECT

public:
    DownloaderItem(const QString &filepath, const QList<QUrl>& urls, const QUrl& danmakuUrl = QUrl(), bool isDash = false, QObject *parent = nullptr);
    virtual void pause(void) override;
    virtual void start(void) override;
    virtual void stop(void) override;
    
private:
    static QList<FileDownloader*> s_waiting;
    static std::atomic<int> s_threadCount;
    static void continueWaitingItems(void);
    
    QList<FileDownloader*> m_downloading;
    QDir m_tempDir;
    QProcess* m_process;
    std::atomic<int> m_finished;
    int m_total;
    bool m_isDash;
    
    void concatVideos(void);
    
private slots:
    void onConcatFinished(int status);
};

#endif // DOWNLOADERMULTIPLEITEM_H
