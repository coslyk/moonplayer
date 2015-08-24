#include "player.h"
#include "ui_player.h"
#include "mplayer.h"
#include "playlist.h"
#include "webvideo.h"
#include "reslibrary.h"
#include "settingsdialog.h"
#include "settings_player.h"
#include "settings_audio.h"
#include "downloader.h"
#include "transformer.h"
#include "skin.h"
#include "utils.h"
#include "cutterbar.h"
#include <QDir>
#include <QMenu>
#include <QMenuBar>
#include <QCloseEvent>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QPalette>
#include <QApplication>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QUrl>
#include <QListWidget>
#include <QMessageBox>
#include <QCoreApplication>
#include <QLabel>
#include <iostream>
#ifdef Q_OS_WIN
#include <direct.h>
#endif

Player::Player(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Player)
{
    std::cout << "Initialize player..." << std::endl;
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
    playlist->installEventFilter(this);

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

    ui->pauseButton->hide();
    ui->progressBar->hide();
    playlist->hide();

    //Add Cutterbar
    cutterbar = new CutterBar;
    int insertPos = ui->mainLayout->indexOf(ui->toolBar);
    ui->mainLayout->insertWidget(insertPos, cutterbar);
    cutterbar->hide();
    mplayer->menu->addAction(tr("Cut video"), this, SLOT(showCutterbar()), QKeySequence("C"));

    //Add WebVideo
    webvideo = new WebVideo;
    reslibrary = new ResLibrary;
    webvideo->insertTab(0, reslibrary, tr("Resources"));
    webvideo->setCurrentIndex(0);

    //add downloader
    downloader = new Downloader;
    webvideo->addTab(downloader, tr("Downloader"));

    //add transformer
    transformer = new Transformer;

    //Settings Dialog
    settingsDialog = new SettingsDialog(this);

    //Add menu
    menubar = new QMenuBar;
    menu = menubar->addMenu(tr("Player"));
    menu->addAction(tr("Online video"), webvideo, SLOT(show()));
    menu->addAction(tr("Transform video"), transformer, SLOT(show()));
    menu->addAction(tr("Settings"), this, SLOT(onSetButton()));
    menu->addSeparator();
    menu->addAction(tr("Homepage"), this, SLOT(openHomepage()));
    ui->mainLayout->insertWidget(0, menubar);

    //Add time show
    timeShow = new QLabel(mplayer);
    timeShow->move(0, 0);
    timeShow->hide();

    //Connect
    connect(ui->playButton, SIGNAL(clicked()), mplayer, SLOT(changeState()));
    connect(ui->pauseButton, SIGNAL(clicked()), mplayer, SLOT(changeState()));
    connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(onStopButton()));
    connect(ui->progressBar, SIGNAL(valueChanged(int)), this, SLOT(onPBarChanged(int)));
    connect(ui->progressBar, SIGNAL(sliderPressed()), this, SLOT(onPBarPressed()));
    connect(ui->progressBar, SIGNAL(sliderReleased()), this, SLOT(onPBarReleased()));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), mplayer, SLOT(setVolume(int)));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(onSaveVolume(int)));
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

    connect(cutterbar, SIGNAL(newFrame(int)), mplayer, SLOT(jumpTo(int)));
    connect(cutterbar, SIGNAL(finished()), cutterbar, SLOT(hide()));
    connect(cutterbar, SIGNAL(finished()), ui->toolBar, SLOT(show()));

    //Set skin
    setSkin(Settings::skinList[Settings::currentSkin]);

    //Set default volume
    ui->volumeSlider->setValue(Settings::volume);

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
        //show borders
        leftBorder->show();
        rightBorder->show();
        bottomBorder->show();

        ui->toolBar->show();
        ui->netButton->setEnabled(true);
    }
    else
    {
        showFullScreen();
        is_fullscreen = true;
        //hide other widgets
        ui->titleBar->hide();
        ui->toolBar->hide();
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
    static bool ctrl_pressed = false;

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

    // Hide or show progressbar, toolbar and playlist
    else if (e->type() == QEvent::Enter && obj == ui->toolBar)
    {
        ui->progressBar->show();
        return true;
    }

    else if (e->type() == QEvent::Leave && obj == ui->toolBar)
    {
        ui->progressBar->hide();
        if (is_fullscreen)
        {
            ui->toolBar->hide();
            toolbar_visible = false;
        }
        return true;
    }

    else if (e->type() == QEvent::Leave && obj == playlist)
    {
        playlist->hide();
        return true;
    }

    else if (e->type() == QEvent::MouseMove)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        if (is_fullscreen && me->y() > toolbar_pos_y && !toolbar_visible) //mouse enters toolbar
        {
            ui->toolBar->show();
            toolbar_visible = true;
        }
        else if (!is_fullscreen && me->x() > width() - 100)
            playlist->show();
        return true;
    }


    //key pressed
    else if (e->type() == QEvent::KeyRelease)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        if (ke->key() == Qt::Key_Control)
        {
            ctrl_pressed = false;
            return true;
        }
        return false;
    }
    else if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        switch (ke->key())
        {
        case Qt::Key_Control:
            ctrl_pressed = true;
            return true;
        case Qt::Key_S:
            mplayer->screenShot();
            return true;
        case Qt::Key_C:
            showCutterbar();
            return true;
        case Qt::Key_Return:
            setFullScreen();
            return true;
        case Qt::Key_F3:
            if (!is_fullscreen)
                hidePlaylist();
            return true;
        case Qt::Key_Space:
            mplayer->changeState();
            return true;
        case Qt::Key_R:
            mplayer->speedSetToDefault();
            return true;
        case Qt::Key_Left: 
            if (ctrl_pressed)
                mplayer->speedDown();
            else
                ui->progressBar->setValue(ui->progressBar->value() - 1);
            return true;

        case Qt::Key_Right:
            if (ctrl_pressed)
                mplayer->speedUp();
            else
                ui->progressBar->setValue(ui->progressBar->value() + 1);
            return true;

        case Qt::Key_Up:
            ui->volumeSlider->setValue(ui->volumeSlider->value() + 1);
            return true;
        case Qt::Key_Down:
            ui->volumeSlider->setValue(ui->volumeSlider->value() - 1);
            return true;
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

//Show Cutterbar
void Player::showCutterbar()
{
    if (mplayer->state == MPlayer::STOPPING || cutterbar->isVisible())
        return;
    QString filename = mplayer->currentFile();
    if (filename.startsWith("http://"))
    {
        QMessageBox::warning(this, "Error", tr("Only support cutting local videos!"));
        return;
    }
    if (is_fullscreen)
        setFullScreen();
    if (mplayer->state == MPlayer::TV_PLAYING || mplayer->state == MPlayer::VIDEO_PLAYING)
        mplayer->changeState();
    ui->toolBar->hide();
    cutterbar->init(filename, mplayer->getLength(), mplayer->getTime());
    cutterbar->show();
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
    int oldSkin = Settings::currentSkin;
    onNeedPause(true);
    settingsDialog->exec();

    int newSkin = Settings::currentSkin;
    if (oldSkin != newSkin)
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
    ui->playButton->show();
    ui->pauseButton->hide();
}

void Player::setIconToPause()
{
    ui->pauseButton->show();
    ui->playButton->hide();
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
    QSize newsize = QSize(sz.width() + 8, height() - mplayer->height() + sz.height());
    QRect available = QApplication::desktop()->availableGeometry();
    if (newsize.width() > available.width() || newsize.height() > available.height())
        setGeometry(available);
    else
        resize(newsize);
}

//ProgressBar

void Player::onPBarPressed()
{
    if (mplayer->state == MPlayer::STOPPING)
        return;
    QString time = secToTime(ui->progressBar->value() * MPlayer::UPDATE_FREQUENCY, true);
    timeShow->setText(time);
    timeShow->show();
    timeShow->move(mplayer->pos());
}

void Player::onPBarChanged(int pos)
{
    if (mplayer->state == MPlayer::STOPPING)
        return;
    if (timeShow->isVisible())  // slider pressed
        timeShow->setText(secToTime(pos * MPlayer::UPDATE_FREQUENCY, true));
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

void Player::onSaveVolume(int volume)
{
    Settings::volume = volume;
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
    // Load skin.qml
    if (chdir(dir.absolutePath().toUtf8().constData()))
    {
        QMessageBox::warning(this, "Error", tr("Failed to read skin!"));
        exit(EXIT_FAILURE);
    }
    QFile qssFile("skin.qss");
    if (!qssFile.open(QFile::ReadOnly | QFile::Text))
    {
        QMessageBox::warning(this, "Error", tr("Failed to read skin!"));
        exit(EXIT_FAILURE);
    }
    QString qss = QString::fromUtf8(qssFile.readAll());
    qssFile.close();
    setStyleSheet(qss);

    // Set fixed sizes
    QFile sizeFile("fixed_sizes");
    if (sizeFile.open(QFile::ReadOnly | QFile::Text))
    {
        QByteArray data = sizeFile.readAll().simplified().replace(" ", "");
        sizeFile.close();
        QStringList sizeInfos = QString::fromUtf8(data).split(';', QString::SkipEmptyParts);
        foreach (QString item, sizeInfos) {
            QStringList properties = item.split(',');
            QWidget *widget = findChild<QWidget*>(properties[0]);
            if (widget)
            {
                int w = properties[1].toInt(), h = properties[2].toInt();
                if (w)
                    widget->setFixedWidth(w);
                if (h)
                    widget->setFixedHeight(h);
                widget->setFocusPolicy(Qt::NoFocus);
            }
        }
    }

    //titlebar and borders
    setWindowFlags(Qt::FramelessWindowHint); //hide borders
    ui->titleBar->show();
    menubar->hide();

    bottomBorder->show();
    leftBorder->show();
    rightBorder->show();
    topLeftBorder->show();
    topRightBorder->show();
}
