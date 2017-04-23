#include "downloaderitem.h"
#include <QUrl>

DownloaderItem::DownloaderItem(const QUrl &url, const QString &filename, QObject *parent) :
    HttpGet(url, filename, parent),
    QTreeWidgetItem((QStringList() << filename << "Wait"))
{
    connect(this, &DownloaderItem::finished, this, &DownloaderItem::onFinished);
    connect(this, &DownloaderItem::paused, this, &DownloaderItem::onPaused);
    connect(this, &DownloaderItem::progressChanged, this, &DownloaderItem::updateProgress);
}

void DownloaderItem::onFinished(HttpGet *, bool error)
{
    setText(1, error ? "Error" : "Finished");
}

void DownloaderItem::onPaused(HttpGet *, int reason)
{
    QString s;
    s.sprintf("Pause (%d)", reason);
    setText(1, s);
}

void DownloaderItem::updateProgress(HttpGet *, int progress, bool isPercentage)
{
    setText(1, QString::number(progress) + (isPercentage ? '%' : 'M'));
}
