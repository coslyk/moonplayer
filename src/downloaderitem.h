#ifndef DOWNLOADERITEM_H
#define DOWNLOADERITEM_H

#include <QObject>
#include <QTreeWidgetItem>


class DownloaderItem : public QObject, public QTreeWidgetItem
{
    Q_OBJECT
public:
    DownloaderItem(const QString &filename, QObject *parent = nullptr);
    virtual void pause(void) = 0;
    virtual void start(void) = 0;
    virtual void stop(void) = 0;

signals:
    void finished(QTreeWidgetItem *item, bool error);
    void paused(int reason);
    void progressChanged(int progress, bool isPercentage);

private slots:
    void onFinished(QTreeWidgetItem *item, bool error);
    void onPaused(int reason);
    void updateProgress(int progress, bool isPercentage);
};

#endif // DOWNLOADERITEM_H
