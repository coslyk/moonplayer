#include "player.h"
#include "ui_player.h"
#include "playercore.h"
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
#ifdef Q_OS_WIN
#include <direct.h>
#endif

Player::Player(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Player)
{
    printf("Initialize player...\n");
    ui->setupUi(this);
    resize(size() * Settings::uiScale);

    //Add PlayerCore frame
    player_core = new PlayerCore;
    ui->playerLayout->setMargin(0);
    ui->playerLayout->addWidget(player_core, 1);
    //enable autohiding toolbar
    player_core->installEventFilter(this);
    player_core->setMouseTracking(true);
    if (player_core->getLayer())
    {
        player_core->getLayer()->installEventFilter(this);
        player_core->getLayer()->setMouseTracking(true);
    }
    ui->toolBar->installEventFilter(this);

    //move window
    ui->titleBar->installEventFilter(this);

    //Add Playlist
    playlist = new Playlist;
    playlist->setMaximumWidth(playlist->maximumWidth() * Settings::uiScale);
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

    //Add WebVideo
    webvideo = new WebVideo;
    webvideo->setMinimumSize(webvideo->minimumSize() * Settings::uiScale);
    reslibrary = new ResLibrary;
    webvideo->insertTab(0, reslibrary, tr("Resources"));
    webvideo->setCurrentIndex(0);

    //add downloader
    downloader = new Downloader;
    webvideo->addTab(downloader, tr("Downloader"));

    //add transformer
    transformer = new Transformer;
    transformer->resize(transformer->size() * Settings::uiScale);

    //Settings Dialog
    settingsDialog = new SettingsDialog(this);

    //Add menu
    menu = new QMenu(tr("Player"), this);
    menu->addAction(tr("Online video"), webvideo, SLOT(show()));
    menu->addAction(tr("Transform video"), transformer, SLOT(show()));
    menu->addAction(tr("Settings"), this, SLOT(onSetButton()));
    menu->addSeparator();
    menu->addAction(tr("Ext. for browser"), this, SLOT(openExtPage()));
    menu->addAction(tr("Homepage"), this, SLOT(openHomepage()));

    //Add time show
    timeShow = new QLabel(player_core);
    timeShow->move(0, 0);
    timeShow->hide();

    //Connect
    connect(ui->playButton, SIGNAL(clicked()), player_core, SLOT(changeState()));
    connect(ui->pauseButton, SIGNAL(clicked()), player_core, SLOT(changeState()));
    connect(ui->stopButton, SIGNAL(clicked()), this, SLOT(onStopButton()));
    connect(ui->progressBar, SIGNAL(valueChanged(int)), this, SLOT(onPBarChanged(int)));
    connect(ui->progressBar, SIGNAL(sliderPressed()), this, SLOT(onPBarPressed()));
    connect(ui->progressBar, SIGNAL(sliderReleased()), this, SLOT(onPBarReleased()));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), player_core, SLOT(setVolume(int)));
    connect(ui->volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(onSaveVolume(int)));
    connect(ui->netButton, SIGNAL(clicked()), webvideo, SLOT(show()));
    connect(ui->closeButton, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui->minButton, SIGNAL(clicked()), this, SLOT(showMinimized()));
    connect(ui->maxButton, SIGNAL(clicked()), this, SLOT(setMaxNormal()));
    connect(ui->menuButton, SIGNAL(clicked()), this, SLOT(showMenu()));

    connect(player_core, SIGNAL(played()), this, SLOT(setIconToPause()));
    connect(player_core, SIGNAL(paused()), this, SLOT(setIconToPlay()));
    connect(player_core, SIGNAL(stopped()), this, SLOT(onStopped()));
    connect(player_core, SIGNAL(timeChanged(int)), this, SLOT(onProgressChanged(int)));
    connect(player_core, SIGNAL(lengthChanged(int)), this, SLOT(onLengthChanged(int)));
    connect(player_core, SIGNAL(fullScreen()), this, SLOT(setFullScreen()));
    connect(player_core, SIGNAL(sizeChanged(const QSize&)), this, SLOT(onSizeChanged(const QSize&)));
    connect(player_core, SIGNAL(cutVideo()), this, SLOT(showCutterbar()));

    connect(playlist, &Playlist::fileSelected, player_core, &PlayerCore::openFile);
    connect(playlist, SIGNAL(needPause(bool)), this, SLOT(onNeedPause(bool)));

    connect(downloader, SIGNAL(newPlay(const QString&,const QString&)), playlist, SLOT(addFileAndPlay(const QString&,const QString&)));
    connect(downloader, SIGNAL(newFile(const QString&,const QString&)), playlist, SLOT(addFile(const QString&,const QString&)));

    connect(cutterbar, SIGNAL(newFrame(int)), player_core, SLOT(jumpTo(int)));
    connect(cutterbar, SIGNAL(finished()), cutterbar, SLOT(hide()));
    connect(cutterbar, SIGNAL(finished()), ui->toolBar, SLOT(show()));

    //Set skin
    setSkin(Settings::skinList[Settings::currentSkin]);

    //Set default volume
    ui->volumeSlider->setValue(Settings::volume);
    ui->volumeSlider->setFixedWidth(100 * Settings::uiScale);

    no_play_next = false;
    mouse_in_toolbar = false;
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
    player_core->stop();
    webvideo->close();
    transformer->close();
    e->accept();
}

void Player::setFullScreen()
{
    if (isFullScreen())
    {
        showNormal();

        //show hidden widgets
        ui->titleBar->show();
        //show borders
        leftBorder->show();
        rightBorder->show();
        bottomBorder->show();

        ui->toolBar->show();
        ui->netButton->setEnabled(true);
    }
    else if (!cutterbar->isVisible()) // Forbidden fullscreen when cutting video
    {
        showFullScreen();
        //hide other widgets
        ui->titleBar->hide();
        ui->toolBar->hide();
        ui->netButton->setEnabled(false);
        //hide borders
        leftBorder->hide();
        rightBorder->hide();
        bottomBorder->hide();
        //set auto-hide toolbar
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
        mouse_in_toolbar = true;
        ui->progressBar->show();
        return true;
    }

    else if (e->type() == QEvent::Leave && obj == ui->toolBar)
    {
        mouse_in_toolbar = false;
        if (!ui->progressBar->isSliderDown())
        {
            ui->progressBar->hide();
            if (isFullScreen())
                ui->toolBar->hide();
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
        if (isFullScreen() && me->y() > toolbar_pos_y && !ui->toolBar->isVisible()) //mouse enters toolbar
            ui->toolBar->show();
        else if (!isFullScreen() && me->x() > width() - 100)
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
            player_core->screenShot();
            return true;
        case Qt::Key_C:
            showCutterbar();
            return true;
        case Qt::Key_Return:
            setFullScreen();
            return true;
        case Qt::Key_Space:
            player_core->changeState();
            return true;
        case Qt::Key_R:
            player_core->speedSetToDefault();
            return true;
        case Qt::Key_Left: 
            if (ctrl_pressed)
                player_core->speedDown();
            else
                ui->progressBar->setValue(ui->progressBar->value() - 1);
            return true;

        case Qt::Key_Right:
            if (ctrl_pressed)
                player_core->speedUp();
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
    if (player_core->state == PlayerCore::STOPPING || player_core->state == PlayerCore::TV_PLAYING || cutterbar->isVisible())
        return;
    QString filename = player_core->currentFile();
    if (filename.startsWith("http://"))
    {
        QMessageBox::warning(this, "Error", tr("Only support cutting local videos!"));
        return;
    }
    if (isFullScreen()) // Exit fullscreen
        setFullScreen();
    if (player_core->state == PlayerCore::VIDEO_PLAYING) //pause
        player_core->changeState();
    ui->toolBar->hide();
    cutterbar->init(filename, player_core->getLength(), player_core->getTime());
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
    player_core->stop();
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

//open extension page
void Player::openExtPage()
{
    static QUrl url("https://github.com/coslyk/moonplayer/wiki/BroswerExtensionZH");
    QDesktopServices::openUrl(url);
}

void Player::onNeedPause(bool b)
{
    if (b && player_core->state == PlayerCore::VIDEO_PLAYING)
        player_core->changeState();
    else if (!b && player_core->state == PlayerCore::VIDEO_PAUSING)
        player_core->changeState();
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
        ui->progressBar->setMaximum(len);
    }
    activateWindow();
    raise();
}

void Player::onSizeChanged(const QSize &sz)
{
    if (isFullScreen() || isMaximized())
        return;
    if (!Settings::autoResize)
        return;
    QSize newsize = QSize(sz.width() + 8 * Settings::uiScale, height() - player_core->height() + sz.height());
    QRect available = QApplication::desktop()->availableGeometry();
    if (newsize.width() > available.width() || newsize.height() > available.height())
        setGeometry(available);
    else
        resize(newsize);
}

//ProgressBar

void Player::onPBarPressed()
{
    if (player_core->state == PlayerCore::STOPPING)
        return;
    QString time = secToTime(ui->progressBar->value(), true);
    timeShow->setText(time);
    timeShow->show();
    timeShow->move(player_core->pos());
}

void Player::onPBarChanged(int pos)
{
    if (player_core->state == PlayerCore::STOPPING)
        return;
    if (timeShow->isVisible())  // slider pressed
        timeShow->setText(secToTime(pos, true));
    else  // move by keyboard
        player_core->setProgress(pos);
}

void Player::onPBarReleased()
{
    if (!mouse_in_toolbar)
    {
        ui->progressBar->hide();
        if (isFullScreen())
            ui->toolBar->hide();
    }
    if (player_core->state == PlayerCore::STOPPING)
        return;
    timeShow->hide();
    player_core->setProgress(ui->progressBar->value());
}

void Player::onProgressChanged(int pos)
{
    ui->progressBar->setValue(pos);
}

void Player::onSaveVolume(int volume)
{
    Settings::volume = volume;
}

void Player::setSkin(const QString& skin_name)
{
    QDir dir = QDir(Settings::path);
    dir.cd("skins");
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
    dir.cd(skin_name);
#elif defined(Q_OS_LINUX)
    if (!dir.cd(skin_name))
        dir = QDir(QDir::homePath() + "/.moonplayer/skins/" + skin_name);
#else
#error ERROR: Unsupported system!
#endif

    // Load skin.qss, skin_normal.qss or skin_hidpi.qss
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
    if (Settings::uiScale > 1.2 && dir.exists("skin_hidpi.qss"))
    {
        QFile f("skin_hidpi.qss");
        f.open(QFile::ReadOnly | QFile::Text);
        qss += QString::fromUtf8(f.readAll());
        f.close();
    }
    else if (dir.exists("skin_normal.qss"))
    {
        QFile f("skin_normal.qss");
        f.open(QFile::ReadOnly | QFile::Text);
        qss += QString::fromUtf8(f.readAll());
        f.close();
    }
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
                    widget->setFixedWidth(w * Settings::uiScale);
                if (h)
                    widget->setFixedHeight(h * Settings::uiScale);
                widget->setFocusPolicy(Qt::NoFocus);
            }
        }
    }

    setWindowFlags(Qt::FramelessWindowHint); //hide borders
}
