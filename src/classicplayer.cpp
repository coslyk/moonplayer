#include "classicplayer.h"
#include "ui_classicplayer.h"
#include "cutterbar.h"
#include "downloader.h"
#include "playercore.h"
#include "playlist.h"
#include "reslibrary.h"
#include "settingsdialog.h"
#include "settings_audio.h"
#include "settings_player.h"
#include "utils.h"
#include "webvideo.h"
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileInfo>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>

ClassicPlayer::ClassicPlayer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClassicPlayer)
{
    printf("Initialize player...\n");

    quit_requested = false;
    no_play_next = false;
    ctrl_pressed = false;

    ui->setupUi(this);
    resize(size() * Settings::uiScale);
    // Set icons
#ifdef Q_OS_MAC
    ui->netButton->setIcon(QIcon(Settings::path + "/icons/net.png"));
    ui->pauseButton->setIcon(QIcon(Settings::path + "/icons/pause.png"));
    ui->playButton->setIcon(QIcon(Settings::path + "/icons/play.png"));
    ui->stopButton->setIcon(QIcon(Settings::path + "/icons/stop.png"));
    ui->volumeButton->setIcon(QIcon(Settings::path + "/icons/volume.png"));
    ui->netButton->setIconSize(QSize(16, 16));
    ui->pauseButton->setIconSize(QSize(16, 16));
    ui->playButton->setIconSize(QSize(16, 16));
    ui->stopButton->setIconSize(QSize(16, 16));
    ui->volumeButton->setIconSize(QSize(16, 16));
    ui->netButton->setFixedSize(QSize(32, 32));
    ui->pauseButton->setFixedSize(QSize(32, 32));
    ui->playButton->setFixedSize(QSize(32, 32));
    ui->stopButton->setFixedSize(QSize(32, 32));
    ui->volumeButton->setFixedSize(QSize(32, 32));
#else
    QPushButton *buttons[] = {ui->playButton, ui->pauseButton, ui->stopButton, ui->volumeButton, ui->netButton};
    for (int i = 0; i < 5; i++)
        buttons[i]->setIconSize(QSize(24, 24) * Settings::uiScale);
#endif

    // Add player_core frame
    player_core = new PlayerCore;
    ui->playerLayout->addWidget(player_core, 1);
    // Enable autohiding playlist
    player_core->installEventFilter(this);
    player_core->setMouseTracking(true);

    // Add playlist
    playlist = new Playlist;
    playlist->initClassicUI();
    playlist->setMaximumWidth(playlist->maximumWidth() * Settings::uiScale);
    ui->playerLayout->addWidget(playlist);
    playlist->installEventFilter(this);
    playlist->hide();

    ui->toolbar->installEventFilter(this);
    ui->pauseButton->hide();

    // Add cutterbar
    cutterbar = new CutterBar;
    ui->mainLayout->addWidget(cutterbar);
    cutterbar->hide();

    // Add WebVideo
    webvideo = new WebVideo;
    webvideo->setMinimumSize(webvideo->minimumSize() * Settings::uiScale);
    ResLibrary *reslibrary = new ResLibrary;
    webvideo->insertTab(0, reslibrary, tr("Resources"));
    webvideo->setCurrentIndex(0);

    // Add downloader
    downloader = new Downloader;
    webvideo->addTab(downloader, tr("Downloader"));

    // Settings
    settingsDialog = new SettingsDialog(this);

    connect(ui->actionAdd_file_s,        &QAction::triggered, playlist,       &Playlist::onAddItem);
    connect(ui->actionAdd_url,           &QAction::triggered, playlist,       &Playlist::onNetItem);
    connect(ui->actionAdd_playlist,      &QAction::triggered, playlist,       &Playlist::onListItem);
    connect(ui->actionOnline_videos,     &QAction::triggered, webvideo,       &WebVideo::show);
    connect(ui->actionSettings,          &QAction::triggered, settingsDialog, &SettingsDialog::exec);
    connect(ui->actionBrowser_extension, &QAction::triggered, this,           &ClassicPlayer::openExtPage);
    connect(ui->actionHomepage,          &QAction::triggered, this,           &ClassicPlayer::openHomepage);
    connect(ui->actionContribute,        &QAction::triggered, this,           &ClassicPlayer::openContributePage);
    connect(ui->actionPlugins,           &QAction::triggered, this,           &ClassicPlayer::openPluginsPage);

    connect(player_core, &PlayerCore::paused,  ui->playButton,  &QPushButton::show);
    connect(player_core, &PlayerCore::paused,  ui->pauseButton, &QPushButton::hide);
    connect(player_core, &PlayerCore::played,  ui->playButton,  &QPushButton::hide);
    connect(player_core, &PlayerCore::played,  ui->pauseButton, &QPushButton::show);
    connect(player_core, &PlayerCore::stopped,       this, &ClassicPlayer::onStopped);
    connect(player_core, &PlayerCore::sizeChanged,   this, &ClassicPlayer::onSizeChanged);
    connect(player_core, &PlayerCore::lengthChanged, this, &ClassicPlayer::onLengthChanged);
    connect(player_core, &PlayerCore::timeChanged,   this, &ClassicPlayer::onProgressChanged);
    connect(player_core, &PlayerCore::fullScreen,    this, &ClassicPlayer::setFullScreen);
    connect(player_core, &PlayerCore::cutVideo,      this, &ClassicPlayer::showCutterbar);

    connect(ui->playButton,   &QPushButton::clicked,    player_core,  &PlayerCore::changeState);
    connect(ui->pauseButton,  &QPushButton::clicked,    player_core,  &PlayerCore::changeState);
    connect(ui->stopButton,   &QPushButton::clicked,    this,     &ClassicPlayer::onStopButton);
    connect(ui->progressBar,  &QSlider::sliderPressed,  this,     &ClassicPlayer::onPBarPressed);
    connect(ui->progressBar,  &QSlider::valueChanged,   this,     &ClassicPlayer::onPBarChanged);
    connect(ui->progressBar,  &QSlider::sliderReleased, this,     &ClassicPlayer::onPBarReleased);
    connect(ui->volumeSlider, &QSlider::valueChanged,   player_core,  &PlayerCore::setVolume);
    connect(ui->volumeSlider, &QSlider::valueChanged,   this,     &ClassicPlayer::saveVolume);
    connect(ui->netButton,    &QPushButton::clicked,    webvideo, &WebVideo::show);

    connect(cutterbar, &CutterBar::newFrame, player_core,     &PlayerCore::jumpTo);
    connect(cutterbar, &CutterBar::finished, cutterbar,   &CutterBar::hide);
    connect(cutterbar, &CutterBar::finished, ui->toolbar, &QWidget::show);

    connect(downloader, SIGNAL(newPlay(const QString&,const QString&)),
            playlist, SLOT(addFileAndPlay(const QString&,const QString&)));
    connect(downloader, SIGNAL(newFile(const QString&,const QString&)),
            playlist, SLOT(addFile(const QString&,const QString&)));
    connect(player_core, SIGNAL(newFile(const QString&,const QString&)),
            playlist, SLOT(addFile(const QString&,const QString&)));

    connect(playlist, &Playlist::fileSelected, player_core, &PlayerCore::openFile);

    ui->volumeSlider->setValue(Settings::volume);
}

ClassicPlayer::~ClassicPlayer()
{
    delete ui;
}

void ClassicPlayer::closeEvent(QCloseEvent *e)
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

    webvideo->close();
    no_play_next = true;

    // It's not safe to quit until mpv is stopped
    if (player_core->state != PlayerCore::STOPPING)
    {
        player_core->stop();
        quit_requested = true;
        e->ignore();
    }
    else
        e->accept();
}

bool ClassicPlayer::eventFilter(QObject *obj, QEvent *e)
{
    // Hide/show playlist and toolbar
    if (e->type() == QEvent::Leave && obj == ui->toolbar && isFullScreen())
    {
        ui->toolbar->hide();
        e->accept();
        return true;
    }
    if (e->type() == QEvent::Leave && obj == playlist)
    {
        playlist->hide();
        e->accept();
        return true;
    }
    else if (e->type() == QEvent::MouseMove && obj == player_core)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        if (isFullScreen() && me->globalY() > toolbar_pos_y && !ui->toolbar->isVisible()) //mouse enters toolbar
        {
            ui->toolbar->show();
        }
        else if (!isFullScreen() && me->x() > width() - 100)
            playlist->show();
        me->accept();
        return true;
    }

    // Key event
    else if (e->type() == QEvent::KeyRelease)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        keyReleaseEvent(ke);
        return true;
    }
    else if (e->type() == QEvent::KeyPress)
    {
        QKeyEvent *ke = static_cast<QKeyEvent*>(e);
        keyPressEvent(ke);
        return true;
    }
    return false;
}


// Keyboard shortcuts
void ClassicPlayer::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Control:
        ctrl_pressed = true;
        break;
    case Qt::Key_S:
        player_core->screenShot();
        break;
    case Qt::Key_C:
        showCutterbar();
        break;
    case Qt::Key_D:
        player_core->switchDanmaku();
        break;
    case Qt::Key_Space:
        player_core->changeState();
        break;
    case Qt::Key_Return:
        setFullScreen();
        break;
    case Qt::Key_R:
        player_core->speedSetToDefault();
        break;
    case Qt::Key_Left:
        if (ctrl_pressed)
            player_core->speedDown();
        else
            ui->progressBar->setValue(ui->progressBar->value() - 5);
        break;

    case Qt::Key_Right:
        if (ctrl_pressed)
            player_core->speedUp();
        else
            ui->progressBar->setValue(ui->progressBar->value() + 5);
        break;

    case Qt::Key_Up:
        ui->volumeSlider->setValue(ui->volumeSlider->value() + 1);
        break;
    case Qt::Key_Down:
        ui->volumeSlider->setValue(ui->volumeSlider->value() - 1);
        break;
    default: break;
    }
    e->accept();
}

void ClassicPlayer::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Control)
        ctrl_pressed = false;
    e->accept();
}


// Drag & drop files
void ClassicPlayer::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void ClassicPlayer::dropEvent(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    bool first = true;
    foreach (QUrl url, urls) {
        if (url.isLocalFile())
        {
            QString file = url.toLocalFile();
            if (first)
            {
                playlist->addFileAndPlay(QFileInfo(file).fileName(), file);
                first = false;
            }
            else
                playlist->addFile(QFileInfo(file).fileName(), file);
        }
        else if (!url.scheme().isEmpty())
            playlist->addUrl(url.toString());
    }
    e->accept();
}

// Full screen
void ClassicPlayer::setFullScreen()
{
    if (isFullScreen()) // Exit fullscreen
        showNormal();
    else if (!cutterbar->isVisible()) // Forbidden fullscreen when cutting video
        showFullScreen();
}

void ClassicPlayer::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
        if (isFullScreen())
        {
#ifndef Q_OS_MAC
            ui->menubar->hide();
#endif
            ui->toolbar->hide();
            ui->netButton->setEnabled(false);
            toolbar_pos_y = QApplication::desktop()->screenGeometry(this).height() - ui->toolbar->height() / 2;
        }
        else
        {
#ifndef Q_OS_MAC
            ui->menubar->show();
#endif
            ui->toolbar->show();
            ui->netButton->setEnabled(true);
        }
    }
    e->accept();
}

void ClassicPlayer::onSizeChanged(const QSize &sz)
{
    if (isFullScreen() || isMaximized())
        return;
    if (!Settings::autoResize)
        return;
    QSize newsize(sz.width(), height() - player_core->height() + sz.height());
    QSize frameNewSize = frameSize() - size() + newsize;
    QRect available = QApplication::desktop()->availableGeometry(this);
    if (frameNewSize.width() > available.width() || frameNewSize.height() > available.height())
    {
        newsize = available.size() + size() - frameSize();
        available.setSize(newsize);
        setGeometry(available);
    }
    else
        resize(newsize);
}

// Progress slider
void ClassicPlayer::onLengthChanged(int len)
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

void ClassicPlayer::onPBarPressed()
{
    if (player_core->state == PlayerCore::STOPPING)
        return;
    QString time = secToTime(ui->progressBar->value());
    player_core->showText(time.toUtf8());
}

void ClassicPlayer::onPBarChanged(int pos)
{
    if (player_core->state == PlayerCore::STOPPING)
        return;
    if (ui->progressBar->isSliderDown())
        player_core->showText(secToTime(ui->progressBar->value()).toUtf8());
    else
        player_core->setProgress(pos);
}

void ClassicPlayer::onPBarReleased()
{
    if (player_core->state == PlayerCore::STOPPING)
        return;
    player_core->setProgress(ui->progressBar->value());
}

void ClassicPlayer::onProgressChanged(int pos)
{
    if (!ui->progressBar->isSliderDown() && pos % 4 == 0) // Make slider easier to drag
        ui->progressBar->setValue(pos);
}

// Save volume
void ClassicPlayer::saveVolume(int volume)
{
    Settings::volume = volume;
}

//Stop playing
void ClassicPlayer::onStopButton()
{
    no_play_next = true;
    player_core->stop();
}

void ClassicPlayer::onStopped()
{
    if (quit_requested)
    {
        quit_requested = false;
        close();
    }
    else if (no_play_next)
    {
        no_play_next = false;
        ui->playButton->show();
        ui->pauseButton->hide();
    }
    else
        playlist->playNext();
}

// Cut video
void ClassicPlayer::showCutterbar()
{
    if (player_core->state == PlayerCore::STOPPING || player_core->state == PlayerCore::TV_PLAYING || cutterbar->isVisible())
        return;
    QString filename = player_core->currentFile();
    if (filename.startsWith("http://"))
    {
        QMessageBox::warning(this, "Error", tr("Only support cutting local videos!"));
        return;
    }
    if (isFullScreen()) //Exit fullscreen
        setFullScreen();
    if (player_core->state == PlayerCore::VIDEO_PLAYING) //pause
        player_core->changeState();
    ui->toolbar->hide();
    cutterbar->init(filename, player_core->getLength(), player_core->getTime());
    cutterbar->show();
}

//open homepage
void ClassicPlayer::openHomepage()
{
    static QUrl url("https://github.com/coslyk/moonplayer");
    QDesktopServices::openUrl(url);
}

//open extension page
void ClassicPlayer::openExtPage()
{
    static QUrl url("https://github.com/coslyk/moonplayer/wiki/BroswerExtension");
    QDesktopServices::openUrl(url);
}

//open contribute page
void ClassicPlayer::openContributePage()
{
    static QUrl url("https://github.com/coslyk/moonplayer/wiki/Contribute");
    QDesktopServices::openUrl(url);
}

//open plugins page
void ClassicPlayer::openPluginsPage()
{
    static QUrl url("https://github.com/coslyk/moonplayer-plugins");
    QDesktopServices::openUrl(url);
}
