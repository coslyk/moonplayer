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
#include "settings_player.h"
#include "settings_plugins.h"
#include "utils.h"
#include "pyapi.h"
#include "ykdlbridge.h"
#include "yougetbridge.h"

class ItemForPlaylist : public QListWidgetItem
{
public:
    QString uri;
    QString danmaku;
    QString audioTrack;
    ItemForPlaylist(const QString &name, const QString &uri, const QString &danmaku, const QString &audioTrack) :
        QListWidgetItem(name), uri(uri), danmaku(danmaku), audioTrack(audioTrack) {}
};

Playlist *playlist = NULL;

Playlist::Playlist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Playlist)
{
    printf("Initialize playlist...\n");
    ui->setupUi(this);
    connect(ui->delButton, SIGNAL(clicked()), this, SLOT(onDelButton()));
    connect(ui->clearButton, SIGNAL(clicked()), this, SLOT(clearList()));

    menu = new QMenu(this);
    menu->addAction(tr("Add file"), this, SLOT(onAddItem()), QKeySequence("Ctrl+O"));
    menu->addAction(tr("Add url"), this, SLOT(onNetItem()), QKeySequence("Ctrl+U"));
    menu->addAction(tr("Add playlist"), this, SLOT(onListItem()));
    connect(ui->addButton, SIGNAL(clicked()), this, SLOT(showMenu()));
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(selectFile(QListWidgetItem*)));

    playlist = this;
}

Playlist::~Playlist()
{
    delete ui;
}

void Playlist::initClassicUI()
{
#ifdef Q_OS_LINUX
    QSize size = QSize(16, 16) * Settings::uiScale;
    ui->addButton->setIcon(QIcon::fromTheme("list-add"));
    ui->addButton->setIconSize(size);
    ui->addButton->setFlat(true);
    ui->delButton->setIcon(QIcon::fromTheme("list-remove"));
    ui->delButton->setIconSize(size);
    ui->delButton->setFlat(true);
    ui->clearButton->setIcon(QIcon::fromTheme("user-trash"));
    ui->clearButton->setIconSize(size);
    ui->clearButton->setFlat(true);
#else
    ui->addButton->setText(" + ");
    ui->delButton->setText(" - ");
    ui->clearButton->setText(" C ");
#endif
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

    if (!files.isEmpty()) // first file
    {
        QString file = files.takeFirst();
        QString name = file.section('/', -1, -1);
        addFileAndPlay(name, file);
    }

    while (!files.isEmpty())
    {
        QString file = files.takeFirst();
        QString name = file.section('/', -1, -1);
        addFile(name, file);
    }

    emit needPause(false);
}

void Playlist::addFile(const QString& name, const QString& file, const QString &danmaku, const QString &audioTrack)
{
    ui->listWidget->addItem(new ItemForPlaylist(name, file, danmaku, audioTrack));
}

void Playlist::addFileAndPlay(const QString& name, const QString& file, const QString &danmaku, const QString &audioTrack)
{
    last_index = ui->listWidget->count();
    ui->listWidget->addItem(new ItemForPlaylist(name, file, danmaku, audioTrack));
    emit fileSelected(file, danmaku, audioTrack);
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
    bool down = (QMessageBox::question(this, "Question", tr("Download?"),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes);
    if (Settings::parser == Settings::YKDL)
        ykdl_bridge.parse(url, down);
    else
        you_get_bridge.parse(url, down);
}

//called when a file is selected
void Playlist::selectFile(QListWidgetItem *item)
{
    last_index = ui->listWidget->row(item);
    ItemForPlaylist *i = static_cast<ItemForPlaylist*>(item);
    emit fileSelected(i->uri, i->danmaku, i->audioTrack);
}

//play the next video
void Playlist::playNext()
{
    last_index++;
    if (last_index < ui->listWidget->count())
    {
        ui->listWidget->setCurrentRow(last_index);
        ItemForPlaylist *item = static_cast<ItemForPlaylist*>(ui->listWidget->item(last_index));
        emit fileSelected(item->uri, item->danmaku, item->audioTrack);
    }
}
