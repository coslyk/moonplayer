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
#include <QHash>
#include "httpget.h"
#include "settings_network.h"
#include "settings_plugins.h"
#include "videocombiner.h"
#include <iostream>

class DownloaderGroup : public QTreeWidgetItem {
public:
    int finished;
    QDir dir;
    DownloaderGroup(QTreeWidget *tree, const QStringList &labels) : QTreeWidgetItem(tree, labels) {}
};

Downloader *downloader = NULL;

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
    treeWidget->setColumnWidth(0, 850);
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


void Downloader::addTask(const QByteArray &url, const QString &filename, bool in_group)
{
    //rename if the same file is exist
    if (QFile::exists(filename))
    {
        if (QMessageBox::question(this, "Download again?", filename + tr(" is existed. Download again?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)
            return;
    }

    HttpGet *get = new HttpGet(QUrl::fromEncoded(url.simplified(), QUrl::StrictMode), filename, this);
    connect(get, SIGNAL(finished(HttpGet*,bool)), this, SLOT(onFinished(HttpGet*,bool)));
    connect(get, SIGNAL(paused(HttpGet*,int)), this, SLOT(onPaused(HttpGet*,int)));
    connect(get, SIGNAL(progressChanged(HttpGet*,int,bool)), this, SLOT(onProgressChanged(HttpGet*,int,bool)));

    //Create item
    QFileInfo info(filename);
    QStringList labels;
    QTreeWidgetItem *item;
    if (in_group)
    {
        DownloaderGroup *group = dir2group[info.path()];
        if (group == NULL)
        {
            labels << info.path() << "";
            group = new DownloaderGroup(treeWidget, labels);
            group->finished = 0;
            group->dir = QDir(info.path());
            dir2group[info.path()] = group;
            labels.clear();
        }
        labels << info.fileName() << "Wait";
        item = new QTreeWidgetItem(group, labels);
        group->setText(1, QString().sprintf("%d / %d", group->finished, group->childCount()));
    }
    else
    {
        labels << filename  << "Wait";
        item = new QTreeWidgetItem(treeWidget, labels);
    }
    get2item[get] = item;
    item2get[item] = get;

    //Start downloading
    if (n_downloading < Settings::maxTasks) {
        get->start();
        n_downloading++;
    }
    else
        waitings << get;
}

void Downloader::onFinished(HttpGet *get, bool error)
{
    QTreeWidgetItem *item = get2item[get];
    if (error)
        item->setText(1, "Error");
    else
    {
        item->setText(1, "Finished");
        if (item->parent())
        {
            DownloaderGroup *group = static_cast<DownloaderGroup*>(item->parent());
            group->finished++;
            group->setText(1, QString().sprintf("%d / %d", group->finished, group->childCount()));
            if (Settings::autoCombine && group->finished == group->childCount())
                new VideoCombiner(this, group->dir);
        }
    }
    get2item.remove(get);
    item2get[item] = NULL;
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
}

void Downloader::onDelButton()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (!item)
        return;

    QString state = item->text(1);
    if (state != "Finished" && state != "Error")
    {
        if (QMessageBox::No == QMessageBox::question(this, "Confirm", tr("File is being downloaded. Still want to delete?"),
            QMessageBox::Yes, QMessageBox::No))
            return;
        HttpGet *get = item2get[item];
        if (get == NULL)
            return;
        get->stop();
        get2item.remove(get);
        if (state == "Wait")
            waitings.removeOne(get);
        else
            n_downloading--;
    }
    item2get.remove(item);
    delete item;
}

void Downloader::onPauseButton()
{
    QTreeWidgetItem *item = treeWidget->currentItem();
    if (!item)
        return;
    HttpGet *get = item2get[item];
    if (get == NULL)
        return;
    if (item->text(1) == "Wait")
    {
        waitings.removeOne(get);
        n_downloading++;
    }
    get->pause();
}

void Downloader::onProgressChanged(HttpGet *get, int progress, bool is_percentage)
{
    QTreeWidgetItem *item = get2item[get];
    if (is_percentage)
        item->setText(1, QString::number(progress) + '%');
    else
        item->setText(1, QString::number(progress) + 'M');
}

void Downloader::onPaused(HttpGet *get, int reason)
{
    QTreeWidgetItem *item = get2item[get];
    QString s;
    s.sprintf("Pause (%d)", reason);
    item->setText(1, s);
}
