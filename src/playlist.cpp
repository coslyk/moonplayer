#include "playlist.h"
#include "ui_playlist.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTimer>
#include <QListWidget>
#include <QMenu>
#include <QFile>
#include <QUrl>
#include "skin.h"
#include "utils.h"
#include "plugins.h"
#include "pyapi.h"
#include <iostream>
#ifdef Q_OS_LINUX
#include <QDBusConnection>
#include <QDebug>
#include <QDBusError>
#endif

class ItemForPlaylist : public QListWidgetItem
{
public:
    QString uri;
    QString danmaku;
    ItemForPlaylist(const QString &name, const QString &uri, const QString &danmaku) :
        QListWidgetItem(name), uri(uri), danmaku(danmaku) {}
};

Playlist *playlist = NULL;

Playlist::Playlist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Playlist)
{
    std::cout << "Initialize playlist..." << std::endl;
    ui->setupUi(this);
    connect(ui->delButton, SIGNAL(clicked()), this, SLOT(onDelButton()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearList()));

    menu = new QMenu(this);
    menu->addAction(tr("Add file"), this, SLOT(onAddItem()));
    menu->addAction(tr("Add url"), this, SLOT(onNetItem()));
    menu->addAction(tr("Add playlist"), this, SLOT(onListItem()));
    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(showMenu()));
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(selectFile(QListWidgetItem*)));

    playlist = this;

#ifdef Q_OS_LINUX
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.registerObject("/", this, QDBusConnection::ExportAllSlots))
    {
        qDebug() << conn.lastError().message();
        exit(EXIT_FAILURE);
    }
#endif
}

Playlist::~Playlist()
{
    delete ui;
}

void Playlist::showMenu()
{
    QPoint pos;
    pos.setY(ui->addButton->height());
    menu->exec(ui->addButton->mapToGlobal(pos));
}

// Delete
void Playlist::onDelButton()
{
    QListWidgetItem* item = ui->listWidget->currentItem();
    if (item == 0)
        return;
    delete item;
}

void Playlist::clearList()
{
    ui->listWidget->clear();
}

// Add
void Playlist::onAddItem()
{
    emit needPause(true);

    QStringList files = QFileDialog::getOpenFileNames(this);
    while (!files.isEmpty())
    {
        QString file = files.takeFirst();
        QString name = file.section('/', -1, -1);
        addFile(name, file);
    }

    emit needPause(false);
}

void Playlist::addFile(const QString& name, const QString& file, const QString &danmaku)
{
    ui->listWidget->addItem(new ItemForPlaylist(name, file, danmaku));
}

void Playlist::addFileAndPlay(const QString& name, const QString& file, const QString &danmaku)
{
    last_index = ui->listWidget->count();
    ui->listWidget->addItem(new ItemForPlaylist(name, file, danmaku));
    emit fileSelected(file, danmaku);
}

// Add list
void Playlist::onListItem()
{
    emit needPause(true);
    QString file = QFileDialog::getOpenFileName(this, 0, 0, "Playlist (*.m3u *m3u8 *.xspf)");
    if (!file.isNull())
        addList(file);
    emit needPause(false);
}

void Playlist::addList(const QString& filename)
{
    QFile file(filename);
    QString line;
    if (!file.open(QFile::ReadOnly | QFile::Text))
        return;
    clearList();

    if (filename.endsWith(".xspf")) //Read .xspf playlist
    {
        QStringList list;
        QByteArray page = file.readAll();
        file.close();
        readXspf(page, list);
        while (!list.isEmpty())
        {
            QString name = list.takeFirst();
            QString file = list.takeFirst();
            addFile(name, file);
        }
        return;
    }

    while (!file.atEnd()) //read .m3u playlist
    {
        line = file.readLine().split('#')[0].simplified();
        if (!line.isEmpty())
            addFile(line.section('/', -1), line);
    }
    file.close();
}

//play Internet resources
void Playlist::onNetItem()
{
    emit needPause(true);
    QString url = QInputDialog::getText(this, tr("Enter url"), tr("Please enter url")).simplified();
    if (!url.isEmpty())
        addUrl(url);
    emit needPause(false);
}

void Playlist::addUrl(const QString &url)
{
    Plugin *plugin = getPluginByHost(QUrl(url).host());
    if (plugin == NULL)
    {
        QString s = url.section('?', 0, 0);
        if (s.endsWith(".html") || s.endsWith(".htm"))
            plugin = flvcd_parser;
    }

    if (plugin)
    {
        if (geturl_obj->hasTask())
        {
            QMessageBox::warning(this, "warning", tr("Another file is parsing. Please wait."));
            return;
        }
        bool down = (QMessageBox::question(this, "Question", tr("Download?"),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes);
        plugin->parse(url.toUtf8().constData(), down);
    }
    else
        addFileAndPlay(url.section('/', -1), url);
}

//called when a file is selected
void Playlist::selectFile(QListWidgetItem *item)
{
    last_index = ui->listWidget->row(item);
    ItemForPlaylist *i = static_cast<ItemForPlaylist*>(item);
    emit fileSelected(i->uri, i->danmaku);
}

//play the next video
void Playlist::playNext()
{
    last_index++;
    if (last_index < ui->listWidget->count())
    {
        ui->listWidget->setCurrentRow(last_index);
        ItemForPlaylist *item = static_cast<ItemForPlaylist*>(ui->listWidget->item(last_index));
        emit fileSelected(item->uri, item->danmaku);
    }
}
