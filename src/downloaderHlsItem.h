#ifndef DOWNLAODERHLSITEM_H
#define DOWNLAODERHLSITEM_H

#include "downloaderAbstractItem.h"

class QProcess;

class DownloaderHlsItem : public DownloaderAbstractItem
{
    Q_OBJECT
    
public:
    DownloaderHlsItem(const QString& filepath, const QUrl& url, QObject* parent = nullptr);
    ~DownloaderHlsItem();
    virtual void pause(void);
    virtual void start(void);
    virtual void stop(bool continueWaiting = true);

private:
    QProcess* m_process;

private slots:
    void readOutput(void);
    void onProcFinished(int code);
};

#endif // DOWNLAODERHLSITEM_H
