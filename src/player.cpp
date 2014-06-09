#include "player.h"
#include "ui_player.h"
#include "mplayer.h"
#include "playlist.h"
#include "webvideo.h"
#include "settings.h"
#include "downloader.h"
#include "transformer.h"
#include "skin.h"
#include <QDir>
#include <QMenu>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QUrl>
#include <QListWidget>
#include <QMessageBox>

static QString secToTime(int second);

Player::Player(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Player)
{
    ui->setupUi(this);

    //Add MPlayer frame
    mplayer = new MPlayer;
    ui->playerLayout->setMargin(0);
    ui->playerLayout->addWidget(mplayer, 1);
    //enable autohiding toolbar
    mplayer->installEventFilter(this);
    mplayer->setMouseTracking(true);
    mplayer->getLayer()->installEventFilter(this);
    mplayer->getLayer()->setMouseTracking(true);
    ui->toolBar->installEventFilter(this);

    //move window
    ui->titleBar->installEventFilter(this);

    //Add Playlist
    playlist = new Playlist;
    ui->playerLayout->addWidget(playlist);

    //Add Border
    topLeftBorder = new Border(this, Border::LEFT);
    topLeftBorder->setObjectName("topLeftBorder");
    topRightBorder = new Border(this, Border::RIGHT);
    topRightBorder->setObjectName("topRightBorder");
    ui->titleBarLayout->insertWidget(0, topLeftBorder);
    ui->titleBarLayout->addWidget(topRightBorder);

    leftBorder = new Border(this, Border::LEFT);
    leftBorder->setObjectName("leftBorder");
    rightBorder = new Border(this, Border::RIGHT);
    rightBorder->setObjectName("rightBorder");
    bottomBorder = new Border(this, Border::BOTTOM);
    bottomBorder->setObjectName("bottomBorder");
    ui->playerLayout->insertWidget(0, leftBorder);
    ui->playerLayout->addWidget(rightBorder);
    ui->mainLayout->addWidget(bottomBorder);

    //Add WebVideo
    webvideo = new WebVideo;

    //add downloader
    downloader = new Downloader;
    webvideo->addTab(downloader, tr("Downloader"));

    //add transformer
    transformer = new Transformer;

    //Add menu
    menu = new QMenu(this);
    menu->addAction(tr("Online video"), webvideo, SLOT(show()));
    menu->addAction(tr("Transform video"), transformer, SLOT(show()));
    menu->addAction(tr("Settings"), this, SLOT(onSetButton()));
    menu->addSeparator();
    menu->addAction(tr("Homepage"), this, SLOT(openHomepage()));

    //Add time show
    timeShow = new QLabel(mplayer);
    timeShow->move(0, 0);
    timeShow->hide();

    //Connect
    connect(ui->playButton, SIGNAL(clicked()), mplayer, SLOT(changeState()));
    connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(onStopButton()));
    connect(ui->progressBar, SIGNAL(valueChanged(int)), this, SLOT(onPBarChanged(int)));
    connect(ui->progressBar, SIGNAL(sliderPressed()), this, SLOT(onPBarPressed()));
    connect(ui->progressBar, SIGNAL(sliderReleased()), this, SLOT(onPBarReleased()));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), mplayer, SLOT(setVolume(int)));
    connect(ui->hideButton, SIGNAL(clicked()), this, SLOT(hidePlaylist()));
    connect(ui->netButton, SIGNAL(clicked()), webvideo, SLOT(show()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->minButton, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(ui->maxButton, SIGNAL(clicked()), this, SLOT(setMaxNormal()));
    connect(ui->menuButton, SIGNAL(clicked()), this, SLOT(showMenu()));

    connect(mplayer, SIGNAL(played()), this, SLOT(setIconToPause()));
    connect(mplayer, SIGNAL(paused()), this, SLOT(setIconToPlay()));
    connect(mplayer, SIGNAL(stopped()), this, SLOT(onStopped()));
    connect(mplayer, SIGNAL(timeChanged(int)), this, SLOT(onProgressChanged(int)));
    connect(mplayer, SIGNAL(lengthChanged(int)), this, SLOT(onLengthChanged(int)));
    connect(mplayer, SIGNAL(fullScreen()), this, SLOT(setFullScreen()));
    connect(mplayer, SIGNAL(sizeChanged(QSize&)), this, SLOT(onSizeChanged(QSize&)));

    connect(playlist, SIGNAL(fileSelected(const QString&)), mplayer, SLOT(openFile(const QString&)));
    connect(playlist, SIGNAL(needPause(bool)), this, SLOT(onNeedPause(bool)));

    connect(downloader, SIGNAL(newPlay(const QString&,const QString&)), playlist, SLOT(addFileAndPlay(const QString&,const QString&)));
    connect(downloader, SIGNAL(newFile(const QString&,const QString&)), playlist, SLOT(addFile(const QString&,const QString&)));

    //Set skin
    setSkin(Settings::skinList[Settings::currentSkin]);

    no_play_next = false;
    is_fullscreen = false;
}

Player::~Player()
{
    delete ui;
}

void Player::closeEvent(QCloseEvent* e)
{
    if (downloader->hasTask())
    {
        bool ignore = (QMessageBox::question(this, "question",
                                         tr("Some files are being downloaded. Do you still want to close?"),
                                         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No);
        if (ignore)
        {
            e->ignore();
            return;
        }
    }
    if (transformer->hasTask())
    {
        bool ignore = (QMessageBox::question(this, "question",
                                         tr("Some files are being transformed. Do you still want to close?"),
                                         QMessageBox::Yes, QMessageBox::No) == QMessageBox::No);
        if (ignore)
        {
            e->ignore();
            return;
        }
    }
    mplayer->stop();
    webvideo->close();
    transformer->close();
    e->accept();
}

//Hide playlist
void Player::hidePlaylist()
{
    if (playlist->isHidden())
        playlist->show();
    else
        playlist->hide();
}

void Player::setFullScreen()
{
    if (is_fullscreen)
    {
        showNormal();
        is_fullscreen = false;
        //show hidden widgets
        ui->titleBar->show();
        ui->toolBar->show();
        playlist->show();
        ui->hideButton->setEnabled(true);
        ui->netButton->setEnabled(true);
        //show borders
        leftBorder->show();
        rightBorder->show();
        bottomBorder->show();
    }
    else
    {
        showFullScreen();
        is_fullscreen = true;
        //hide other widgets
        ui->toolBar->hide();
        ui->titleBar->hide();
        playlist->hide();
        ui->hideButton->setEnabled(false);
        ui->netButton->setEnabled(false);
        //hide borders
        leftBorder->hide();
        rightBorder->hide();
        bottomBorder->hide();
        //set auto-hide toolbar
        toolbar_visible = false;
        toolbar_pos_y = QApplication::desktop()->height() - ui->toolBar->height() / 2;
    }
}


bool Player::eventFilter(QObject *obj, QEvent *e)
{
    static QPoint windowPos, mousePos, dPos;

    //Move window
    if (obj == ui->titleBar)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        if (e->type() == QEvent::MouseButtonPress)
        {
            windowPos = pos();
            mousePos = me->globalPos();
            dPos = mousePos - windowPos;
        }
        else if (e->type() == QEvent::MouseMove)
            move(me->globalPos() - dPos);
        return false;
    }

    // Hide or show toolbar in fullscreen
    else if (e->type() == QEvent::Leave && obj == ui->toolBar && is_fullscreen)
    {
        ui->toolBar->hide();
        toolbar_visible = false;
        return true;
    }

    else if (e->type() == QEvent::MouseMove && is_fullscreen)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        if (me->y() > toolbar_pos_y && !toolbar_visible) //mouse enters toolbar
        {
            ui->toolBar->show();
            toolbar_visible = true;
            return true;
        }
    }

    //key pressed
    else if (e->type() == QEvent::KeyPress)
    {
        if (mplayer->state == MPlayer::STOPPING)
            return false;
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        switch (ke->key())
        {
        case Qt::Key_S:
            mplayer->screenShot();
            return true;
        case Qt::Key_Space:
            mplayer->changeState();
            return true;
        case Qt::Key_Left: 
            ui->progressBar->setValue(ui->progressBar->value() - 1);
            return true;
        case Qt::Key_Right:
            ui->progressBar->setValue(ui->progressBar->value() + 1);
            return true;
        case Qt::Key_Up:
            ui->volumeSlider->setValue(ui->volumeSlider->value() + 1);
            return true;
        case Qt::Key_Down:
            ui->volumeSlider->setValue(ui->volumeSlider->value() - 1);
        default:return false;
        }
    }
    return false;
}

//show menu
void Player::showMenu()
{
    QPoint pos;
    pos.setY(ui->menuButton->height());
    menu->exec(ui->menuButton->mapToGlobal(pos));
}

//change between max / normal window
void Player::setMaxNormal()
{
    if (windowState() == Qt::WindowMaximized)
        showNormal();
    else
        showMaximized();
}

//Stop playing
void Player::onStopButton()
{
    no_play_next = true;
    mplayer->stop();
}

void Player::onStopped()
{
    setIconToPlay();
    if (no_play_next)
        no_play_next = false;
    else
        playlist->playNext();
}

//open setting dialog
void Player::onSetButton()
{
    static Settings* dialog = NULL;
    int oldSkin = Settings::currentSkin;
    if (dialog == NULL)
        dialog = new Settings(this);
    onNeedPause(true);
    dialog->exec();
    if (oldSkin != Settings::currentSkin)
    {
        setSkin(Settings::skinList[Settings::currentSkin]);
        show();
    }
    onNeedPause(false);
}

//open homepage
void Player::openHomepage()
{
    static QUrl url("https://github.com/coslyk/moonplayer");
    QDesktopServices::openUrl(url);
}

void Player::onNeedPause(bool b)
{
    if (b && mplayer->state == MPlayer::VIDEO_PLAYING)
        mplayer->changeState();
    else if (!b && mplayer->state == MPlayer::VIDEO_PAUSING)
        mplayer->changeState();
}

void Player::setIconToPlay()
{
    ui->playButton->setIcon(skin[SKIN_PLAY]);
}

void Player::setIconToPause()
{
    ui->playButton->setIcon(skin[SKIN_PAUSE]);
}

void Player::onLengthChanged(int len)
{
    if (len == 0) //playing TV
        ui->progressBar->setEnabled(false);
    else //playing video
    {
        ui->progressBar->setEnabled(true);
        ui->progressBar->setMaximum(len / MPlayer::UPDATE_FREQUENCY);
    }
    activateWindow();
    raise();
}

void Player::onSizeChanged(QSize &sz)
{
    if (is_fullscreen)
        return;
    if (!Settings::autoResize)
        return;
    QSize newsize = size() - mplayer->size() + sz;
    QRect available = QApplication::desktop()->availableGeometry();
    if (newsize.width() > available.width() || newsize.height() > available.height())
        setGeometry(available);
    else
        resize(newsize);
}

//ProgressBar
static QString secToTime(int second)
{
    static QString format = "<span style=\" font-size:14pt; font-weight:600;color:#00ff00;\">%1:%2:%3</span>";
    QString  hour = QString::number(second / 3600);
    QString min = QString::number((second % 3600) / 60);
    QString sec = QString::number(second % 60);
    if (min.length() == 1)
        min.prepend('0');
    if (sec.length() == 1)
        sec.prepend('0');
    return format.arg(hour, min, sec);
}

void Player::onPBarPressed()
{
    if (mplayer->state == MPlayer::STOPPING)
        return;
    QString time = secToTime(ui->progressBar->value() * MPlayer::UPDATE_FREQUENCY);
    timeShow->setText(time);
    timeShow->show();
    timeShow->move(mplayer->pos());
}

void Player::onPBarChanged(int pos)
{
    if (mplayer->state == MPlayer::STOPPING)
        return;
    if (timeShow->isVisible())  // slider pressed
        timeShow->setText(secToTime(pos * MPlayer::UPDATE_FREQUENCY));
    else  // move by keyboard
        mplayer->setProgress(pos * MPlayer::UPDATE_FREQUENCY);
}

void Player::onPBarReleased()
{
    if (mplayer->state == MPlayer::STOPPING)
        return;
    timeShow->hide();
    mplayer->setProgress(ui->progressBar->value() * MPlayer::UPDATE_FREQUENCY);
}

void Player::onProgressChanged(int pos)
{
    ui->progressBar->setValue(pos / MPlayer::UPDATE_FREQUENCY);
}

void Player::setSkin(const QString& skin_name)
{
    QDir dir = QDir(Settings::path);
    dir.cd("skins");
#ifdef Q_OS_WIN
    dir.cd(skin_name);
#else
    if (!dir.cd(skin_name))
        dir = QDir(QDir::homePath() + "/.moonplayer/skins/" + skin_name);
#endif

    //buttons
    skin[SKIN_PLAY] = QPixmap(dir.filePath("play.png"));
    skin[SKIN_STOP] = QPixmap(dir.filePath("stop.png"));
    skin[SKIN_PAUSE] = QPixmap(dir.filePath("pause.png"));
    Skin::setButton(ui->playButton, skin[SKIN_PLAY]);
    Skin::setButton(ui->stopButton, skin[SKIN_STOP]);
    Skin::setButton(ui->closeButton, dir.filePath("close.png"), dir.filePath("close_over.png"));
    Skin::setButton(ui->maxButton, dir.filePath("max.png"), dir.filePath("max_over.png"));
    Skin::setButton(ui->minButton, dir.filePath("min.png"), dir.filePath("min_over.png"));
    Skin::setButton(ui->hideButton, dir.filePath("playlist.png"), dir.filePath("playlist_over.png"));
    Skin::setButton(ui->netButton, dir.filePath("net.png"), dir.filePath("net_over.png"));
    Skin::setButton(ui->menuButton, dir.filePath("menu.png"), dir.filePath("menu_over.png"), dir.filePath("menu_over.png"), tr("Menu"));

    //playlist, volumn icon and toolbar
    playlist->setSkin(dir);
    ui->volumePic->setPixmap(QPixmap(dir.filePath("volume.png")));
    Skin::setWidgetBG(ui->toolBar, dir.filePath("bottom.png"));

    //titlebar and borders
    Skin::setWidgetBG(ui->titleBar, dir.filePath("top.png"));
    if (dir.exists("top.png"))
    {
        setWindowFlags(Qt::FramelessWindowHint); //hide borders
        ui->closeButton->show();
        ui->minButton->show();
        ui->maxButton->show();
    }
    else
    {
        setWindowFlags(0);
        ui->closeButton->hide();
        ui->minButton->hide();
        ui->maxButton->hide();
    }
    bottomBorder->setPicture(QPixmap(dir.filePath("bottom_border.png")));
    leftBorder->setPicture(dir.filePath("left_border.png"));
    rightBorder->setPicture(dir.filePath("right_border.png"));
    Skin::setWidgetBG(topLeftBorder, dir.filePath("topleft_border.png"));
    Skin::setWidgetBG(topRightBorder, dir.filePath("topright_border.png"));

    //sliders
    Skin::setSlider(ui->progressBar, dir.filePath("slider-bg.png"), dir.filePath("slider.png"));
    Skin::setSlider(ui->volumeSlider, dir.filePath("slider-bg.png"), dir.filePath("slider.png"));
}
