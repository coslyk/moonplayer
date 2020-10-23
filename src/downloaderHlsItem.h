#ifndef DOWNLAODERHLSITEM_H
#define DOWNLAODERHLSITEM_H

#include "downloaderAbstractItem.h"
#include <QProcess>

class DownloaderHlsItem : public DownloaderAbstractItem
{
    Q_OBJECT
    
public:
    DownloaderHlsItem(const QString& filepath, const QUrl& url, const QUrl& danmakuUrl = QUrl(), QObject* parent = nullptr);
    ~DownloaderHlsItem();
    virtual void pause(void) override;
    virtual void start(void) override;
    virtual void stop(void) override;

private:
    QProcess m_process;

private slots:
    void readOutput(void);
    void onProcFinished(int code);
};

#endif // DOWNLAODERHLSITEM_H
