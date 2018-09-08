#include "downloaderitem.h"
#include <QUrl>

DownloaderItem::DownloaderItem(const QString &filename, QObject *parent) :
    QObject(parent), QTreeWidgetItem((QStringList() << filename << "Wait"))
{
    connect(this, &DownloaderItem::finished, this, &DownloaderItem::onFinished);
    connect(this, &DownloaderItem::paused, this, &DownloaderItem::onPaused);
    connect(this, &DownloaderItem::progressChanged, this, &DownloaderItem::updateProgress);
}

void DownloaderItem::onFinished(QTreeWidgetItem *, bool error)
{
    setText(1, error ? "Error" : "Finished");
}

void DownloaderItem::onPaused(int reason)
{
    QString s;
    s.sprintf("Pause (%d)", reason);
    setText(1, s);
}

void DownloaderItem::updateProgress(int progress, bool isPercentage)
{
    setText(1, QString::number(progress) + (isPercentage ? '%' : 'M'));
}
