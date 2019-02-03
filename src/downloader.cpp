#include "downloader.h"
#include <QGridLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QSpacerItem>
#include <QMessageBox>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include "httpget.h"
#include "settings_network.h"
#include "streamget.h"
#include "videocombiner.h"
#include <iostream>

class DownloaderGroup : public QTreeWidgetItem {
public:
    int finished;
    QDir dir;
    DownloaderGroup(QTreeWidget *tree, const QStringList &labels) : QTreeWidgetItem(tree, labels) {}
};

Downloader *downloader = nullptr;

Downloader::Downloader(QWidget *parent) :
    QWidget(parent)
{
    std::cout << "Initialize downloader..." << std::endl;
    n_downloading = 0;
    QStringList labels;
    labels << tr("File name") << tr("State");
    treeWidget = new QTreeWidget;
    treeWidget->setColumnCount(2);
    treeWidget->setHeaderLabels(labels);
    treeWidget->setColumnWidth(0, 750);
    treeWidget->setColumnWidth(1, 50);
    QPushButton *playButton = new QPushButton(tr("Play"));
    QPushButton *delButton = new QPushButton(tr("Delete"));
    QPushButton *pauseButton = new QPushButton(tr("Pause"));
    QSpacerItem *spacer = new QSpacerItem(40, 20, QSizePolicy::Expanding);

    QGridLayout *layout = new QGridLayout(this);
    layout->addItem(spacer, 0, 0, 1, 1);
    layout->addWidget(playButton, 0, 1, 1, 1);
    layout->addWidget(delButton, 0, 2, 1, 1);
    layout->addWidget(pauseButton, 0, 3, 1, 1);
    layout->addWidget(treeWidget, 1, 0, 1, 4);

    setWindowTitle(tr("Download Manager"));

    connect(playButton, SIGNAL(clicked()), this, SLOT(onPlayButton()));
    connect(delButton, SIGNAL(clicked()), this, SLOT(onDelButton()));
    connect(pauseButton, SIGNAL(clicked()), this, SLOT(onPauseButton()));

    downloader = this;
}


void Downloader::addTask(const QByteArray &url, const QString &filename, bool in_group, const QByteArray &danmaku)
{
    //rename if the same file is exist
    if (QFile::exists(filename))
    {
        if (QMessageBox::question(this,
                                  "Download again?",
                                  filename + tr(" is existed. Download again?"),
                                  QMessageBox::Yes,
                                  QMessageBox::No) == QMessageBox::No)
            return;
    }

    DownloaderItem *item;
    if (filename.endsWith(".m3u") || filename.endsWith(".m3u8")) // stream
        item = new StreamGet(QString::fromUtf8(url.simplified()), filename.section('.', 0, -2) + ".mp4", this);
    else
        item = new HttpGet(QString::fromUtf8(url.simplified()), filename, this);
    connect(item, &DownloaderItem::finished, this, &Downloader::onFinished);

    // Add item to tree
    if (in_group)
    {
        QFileInfo info(filename);
        DownloaderGroup *group = dir2group[info.path()];
        if (group == nullptr)
        {
            QStringList labels;
            labels << info.path() << "";
            group = new DownloaderGroup(treeWidget, labels);
            group->finished = 0;
            group->dir = QDir(info.path());
            dir2group[info.path()] = group;
        }
        group->addChild(item);
        group->setText(1, QString().sprintf("%d / %d", group->finished, group->childCount()));
    }
    else
        treeWidget->addTopLevelItem(item);

    //save danmaku's url
    if (!danmaku.isEmpty())
    {
        QFile file(filename + ".danmaku");
        if (file.open(QFile::WriteOnly))
        {
            file.write(danmaku);
            file.close();
        }
    }

    //Start downloading
    if (n_downloading < Settings::maxTasks) {
        item->start();
        n_downloading++;
    }
    else
        waitings << item;
}

void Downloader::onFinished(QTreeWidgetItem *item, bool error)
{
    if (!error && item->parent()) //in group
    {
        DownloaderGroup *group = static_cast<DownloaderGroup*>(item->parent());
        group->finished++;
        group->setText(1, QString().sprintf("%d / %d", group->finished, group->childCount()));
        if (Settings::autoCombine && group->finished == group->childCount())
            new VideoCombiner(this, group->dir);
    }
    n_downloading--;
    while (n_downloading < Settings::maxTasks && !waitings.isEmpty())
    {
        waitings.takeFirst()->start();
        n_downloading++;
    }
}

void Downloader::onPlayButton()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (!item)
        return;

    if (item->childCount()) //dir
    {
        QTreeWidgetItem *child = item->child(0);
        QDir dir(item->text(0));
        QString name = child->text(0);
        emit newPlay(name, dir.filePath(name));
        int i = 1;
        child = item->child(1);
        while (child)
        {
            name = child->text(0);
            emit newFile(name, dir.filePath(name));
            i++;
            child = item->child(i);
        }
        window()->close();
        return;
    }
    QString name = item->text(0);
    if (item->parent())
    {
        QDir dir(item->parent()->text(0));
        emit newPlay(name, dir.filePath(name));
    }
    else
        emit newPlay(QFileInfo(name).fileName(), name);
    window()->close();
}

void Downloader::onDelButton()
{
    QTreeWidgetItem *i = treeWidget->currentItem();
    if (!i)
        return;
    if (i->childCount()) //group
        return;

    DownloaderItem *item = static_cast<DownloaderItem*>(i);
    QString state = item->text(1);
    if (state != "Finished" && state != "Error")
    {
        if (QMessageBox::No == QMessageBox::question(this,
                                                     "Confirm",
                                                     tr("File is being downloaded. Still want to delete?"),
                                                     QMessageBox::Yes,
                                                     QMessageBox::No))
            return;
        item->stop();
        if (state == "Wait")
            waitings.removeOne(item);
        else
            n_downloading--;
    }
    item->deleteLater();
}

void Downloader::onPauseButton()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (!item)
        return;
    if (item->childCount() || item->text(1) == "Finished") //group or finished
        return;
    DownloaderItem *i = static_cast<DownloaderItem*>(item);
    if (i->text(1) == "Wait")
    {
        waitings.removeOne(i);
        n_downloading++;
    }
    i->pause();
}
