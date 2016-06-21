#include "classicplayer.h"
#include "ui_classicplayer.h"
#include "cutterbar.h"
#include "downloader.h"
#include "mplayer.h"
#include "playlist.h"
#include "reslibrary.h"
#include "settingsdialog.h"
#include "settings_audio.h"
#include "settings_player.h"
#include "transformer.h"
#include "utils.h"
#include "webvideo.h"
#include <iostream>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QMouseEvent>

ClassicPlayer::ClassicPlayer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::ClassicPlayer)
{
    std::cout << "Initialize player..." << std::endl;
    ui->setupUi(this);
    resize(size() * Settings::uiScale);
    QPushButton *buttons[] = {ui->playButton, ui->pauseButton, ui->stopButton, ui->volumeButton, ui->netButton};
    for (int i = 0; i < 5; i++)
        buttons[i]->setIconSize(QSize(24, 24) * Settings::uiScale);

    // Add mplayer frame
    mplayer = new MPlayer;
    ui->playerLayout->addWidget(mplayer, 1);
    // Enable autohiding playlist
    mplayer->installEventFilter(this);
    mplayer->setMouseTracking(true);
    mplayer->getLayer()->installEventFilter(this);
    mplayer->getLayer()->setMouseTracking(true);

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
    mplayer->menu->addAction(tr("Cut video"), this, SLOT(showCutterbar()), QKeySequence("C"));

    // Add WebVideo
    webvideo = new WebVideo;
    webvideo->setMinimumSize(webvideo->minimumSize() * Settings::uiScale);
    ResLibrary *reslibrary = new ResLibrary;
    webvideo->insertTab(0, reslibrary, tr("Resources"));
    webvideo->setCurrentIndex(0);

    // Add downloader
    downloader = new Downloader;
    webvideo->addTab(downloader, tr("Downloader"));

    // Add transformer
    transformer = new Transformer;
    transformer->resize(transformer->size() * Settings::uiScale);

    // Settings
    settingsDialog = new SettingsDialog(this);

    connect(ui->actionAdd_file_s,        &QAction::triggered, playlist,       &Playlist::onAddItem);
    connect(ui->actionAdd_url,           &QAction::triggered, playlist,       &Playlist::onNetItem);
    connect(ui->actionAdd_playlist,      &QAction::triggered, playlist,       &Playlist::onListItem);
    connect(ui->actionOnline_videos,     &QAction::triggered, webvideo,       &WebVideo::show);
    connect(ui->actionTranscoder,        &QAction::triggered, transformer,    &Transformer::show);
    connect(ui->actionSettings,          &QAction::triggered, settingsDialog, &SettingsDialog::exec);
    connect(ui->actionBrowser_extension, &QAction::triggered, this,           &ClassicPlayer::openExtPage);
    connect(ui->actionHomepage,          &QAction::triggered, this,           &ClassicPlayer::openHomepage);

    connect(mplayer, &MPlayer::paused,  ui->playButton,  &QPushButton::show);
    connect(mplayer, &MPlayer::paused,  ui->pauseButton, &QPushButton::hide);
    connect(mplayer, &MPlayer::played,  ui->playButton,  &QPushButton::hide);
    connect(mplayer, &MPlayer::played,  ui->pauseButton, &QPushButton::show);
    connect(mplayer, &MPlayer::stopped,       this, &ClassicPlayer::onStopped);
    connect(mplayer, &MPlayer::sizeChanged,   this, &ClassicPlayer::onSizeChanged);
    connect(mplayer, &MPlayer::lengthChanged, this, &ClassicPlayer::onLengthChanged);
    connect(mplayer, &MPlayer::timeChanged,   this, &ClassicPlayer::onProgressChanged);
    connect(mplayer, &MPlayer::fullScreen,    this, &ClassicPlayer::setFullScreen);

    connect(ui->playButton,   &QPushButton::clicked,    mplayer,  &MPlayer::changeState);
    connect(ui->pauseButton,  &QPushButton::clicked,    mplayer,  &MPlayer::changeState);
    connect(ui->stopButton,   &QPushButton::clicked,    this,     &ClassicPlayer::onStopButton);
    connect(ui->progressBar,  &QSlider::sliderPressed,  this,     &ClassicPlayer::onPBarPressed);
    connect(ui->progressBar,  &QSlider::valueChanged,   this,     &ClassicPlayer::onPBarChanged);
    connect(ui->progressBar,  &QSlider::sliderReleased, this,     &ClassicPlayer::onPBarReleased);
    connect(ui->volumeSlider, &QSlider::valueChanged,   mplayer,  &MPlayer::setVolume);
    connect(ui->volumeSlider, &QSlider::valueChanged,   this,     &ClassicPlayer::saveVolume);
    connect(ui->netButton,    &QPushButton::clicked,    webvideo, &WebVideo::show);

    connect(cutterbar, &CutterBar::newFrame, mplayer,     &MPlayer::jumpTo);
    connect(cutterbar, &CutterBar::finished, cutterbar,   &CutterBar::hide);
    connect(cutterbar, &CutterBar::finished, ui->toolbar, &QWidget::show);

    connect(downloader, SIGNAL(newPlay(const QString&,const QString&)),
            playlist, SLOT(addFileAndPlay(const QString&,const QString&)));
    connect(downloader, SIGNAL(newFile(const QString&,const QString&)),
            playlist, SLOT(addFile(const QString&,const QString&)));

    connect(playlist, &Playlist::fileSelected, mplayer, &MPlayer::openFile);

    ui->volumeSlider->setValue(Settings::volume);
    no_play_next = false;
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

bool ClassicPlayer::eventFilter(QObject *obj, QEvent *e)
{
    static bool ctrl_pressed = false;

    // Hide/show playlist and toolbar
    if (e->type() == QEvent::Enter && obj == mplayer && isFullScreen())
    {
        ui->toolbar->hide();
        ui->statusBar->hide();
        return true;
    }
    if (e->type() == QEvent::Leave && obj == playlist)
    {
        playlist->hide();
        return true;
    }
    else if (e->type() == QEvent::MouseMove)
    {
        QMouseEvent* me = static_cast<QMouseEvent*>(e);
        if (isFullScreen() && me->y() > toolbar_pos_y && !ui->toolbar->isVisible()) //mouse enters toolbar
        {
            ui->toolbar->show();
            ui->statusBar->show();
        }
        else if (!isFullScreen() && me->x() > width() - 100)
            playlist->show();
        return true;
    }

    // Key event
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
        case Qt::Key_Space:
            mplayer->changeState();
            return true;
        case Qt::Key_Return:
            setFullScreen();
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

// Full screen
void ClassicPlayer::setFullScreen()
{
    if (isFullScreen()) // Exit fullscreen
    {
        showNormal();
        ui->toolbar->show();
        ui->menubar->show();
        ui->statusBar->show();
        ui->netButton->setEnabled(true);
    }
    else if (!cutterbar->isVisible()) // Forbidden fullscreen when cutting video
    {
        showFullScreen();
        ui->toolbar->hide();
        ui->menubar->hide();
        ui->statusBar->hide();
        ui->netButton->setEnabled(false);
        toolbar_pos_y = QApplication::desktop()->height() - ui->toolbar->height() / 2;
    }
}

void ClassicPlayer::onSizeChanged(QSize &sz)
{
    if (isFullScreen() || isMaximized())
        return;
    if (!Settings::autoResize)
        return;
    QSize newsize(sz.width(), height() - mplayer->height() + sz.height());
    QRect available = QApplication::desktop()->availableGeometry();
    if (newsize.width() > available.width() || newsize.height() > available.height())
        setGeometry(available);
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
        ui->progressBar->setMaximum(len / MPlayer::UPDATE_FREQUENCY);
    }
    activateWindow();
    raise();
}

void ClassicPlayer::onPBarPressed()
{
    if (mplayer->state == MPlayer::STOPPING)
        return;
    QString time = secToTime(ui->progressBar->value() * MPlayer::UPDATE_FREQUENCY);
    ui->statusBar->showMessage(time);
}

void ClassicPlayer::onPBarChanged(int pos)
{
    if (mplayer->state == MPlayer::STOPPING)
        return;
    if (ui->progressBar->isSliderDown())
        ui->statusBar->showMessage(secToTime(ui->progressBar->value() * MPlayer::UPDATE_FREQUENCY));
    else
        mplayer->setProgress(pos * MPlayer::UPDATE_FREQUENCY);
}

void ClassicPlayer::onPBarReleased()
{
    if (mplayer->state == MPlayer::STOPPING)
        return;
    mplayer->setProgress(ui->progressBar->value() * MPlayer::UPDATE_FREQUENCY);
    ui->statusBar->clearMessage();
}

void ClassicPlayer::onProgressChanged(int pos)
{
    ui->progressBar->setValue(pos / MPlayer::UPDATE_FREQUENCY);
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
    mplayer->stop();
}

void ClassicPlayer::onStopped()
{
    if (no_play_next)
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
    if (mplayer->state == MPlayer::STOPPING || mplayer->state == MPlayer::TV_PLAYING || cutterbar->isVisible())
        return;
    QString filename = mplayer->currentFile();
    if (filename.startsWith("http://"))
    {
        QMessageBox::warning(this, "Error", tr("Only support cutting local videos!"));
        return;
    }
    if (isFullScreen()) //Exit fullscreen
        setFullScreen();
    if (mplayer->state == MPlayer::VIDEO_PLAYING) //pause
        mplayer->changeState();
    ui->toolbar->hide();
    cutterbar->init(filename, mplayer->getLength(), mplayer->getTime());
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
    static QUrl url("https://github.com/coslyk/moonplayer/wiki/BroswerExtensionZH");
    QDesktopServices::openUrl(url);
}
