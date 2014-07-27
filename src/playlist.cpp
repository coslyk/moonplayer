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
#include "parser.h"
#include "plugins.h"
#include "pyapi.h"
#ifdef Q_OS_LINUX
#include <QDBusConnection>
#include <QDebug>
#include <QDBusError>
#endif

Playlist *playlist = NULL;

Playlist::Playlist(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Playlist)
{
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
    if (!conn.registerObject("/", this, QDBusConnection::ExportNonScriptableSlots))
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

void Playlist::setSkin(const QDir &dir)
{
    Skin::setButton(ui->addButton, dir.filePath("add.png"), dir.filePath("add_over.png"));
    Skin::setButton(ui->delButton, dir.filePath("del.png"), dir.filePath("del_over.png"));
    Skin::setButton(ui->clearButton, dir.filePath("clear.png"), dir.filePath("clear_over.png"));
    Skin::setListView(ui->listWidget, dir.filePath("playlist_bg2.png"),
                      dir.filePath("listitem_selected.png"),
                      dir.filePath("listitem_inactive.png"),
                      dir.filePath("playlist_scrollbar.png"),
                      dir.filePath("playlist_scrollbg.png"),
                      dir.filePath("playlist_hscrollbar.png"),
                      dir.filePath("playlist_hscrollbg.png"));

    QPixmap img(dir.filePath("playlist_bg.png"));
    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), QBrush(img));
    setPalette(pal);
}

void Playlist::setNoSkin()
{
    QSize size(18, 18);
    Skin::setButton(ui->addButton, QIcon::fromTheme("list-add"), size);
    Skin::setButton(ui->delButton, QIcon::fromTheme("list-remove"), size);
    Skin::setButton(ui->clearButton, QIcon::fromTheme("edit-clear"), size);
    ui->listWidget->setStyleSheet(0);
    setAutoFillBackground(false);
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
    filelist.remove(ui->listWidget->row(item));
    delete item;
}

void Playlist::clearList()
{
    ui->listWidget->clear();
    filelist.clear();
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

void Playlist::addFile(const QString& name, const QString& file)
{
    filelist.append(file);
    ui->listWidget->addItem(new QListWidgetItem(name));
}

void Playlist::addFileAndPlay(const QString& name, const QString& file)
{
    last_index = filelist.size();
    filelist.append(file);
    ui->listWidget->addItem(new QListWidgetItem(name));
    emit fileSelected(file);
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
        Parser::readXspf(page, list);
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
    if (plugin)
    {
        if (geturl_obj->hasTask())
        {
            QMessageBox::warning(this, "warning", tr("Another file is parsing. Please wait."));
            return;
        }
        bool down = (QMessageBox::question(this, plugin->getName(), tr("Download?"),
                              QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes);
        plugin->parse(url.toUtf8().constData(), down);
    }
    else
        addFileAndPlay(url.section('/', -1), url);
}

//called when a file is selected
void Playlist::selectFile(QListWidgetItem *item)
{
    int i = ui->listWidget->row(item);
    last_index = i;
    emit fileSelected(filelist[i]);
}

//play the next video
void Playlist::playNext()
{
    last_index++;
    if (last_index < filelist.size())
    {
        ui->listWidget->setCurrentRow(last_index);
        emit fileSelected(filelist[last_index]);
    }
}
