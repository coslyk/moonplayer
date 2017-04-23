#ifndef DOWNLOADERITEM_H
#define DOWNLOADERITEM_H

#include "httpget.h"
#include <QTreeWidgetItem>


class DownloaderItem : public HttpGet, public QTreeWidgetItem
{
    Q_OBJECT
public:
    DownloaderItem(const QUrl &url, const QString &filename, QObject *parent = NULL);

private slots:
    void onFinished(HttpGet *, bool error);
    void onPaused(HttpGet *, int reason);
    void updateProgress(HttpGet *, int progress, bool isPercentage);
};

#endif // DOWNLOADERITEM_H
