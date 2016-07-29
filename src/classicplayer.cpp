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
#include "transformer.h"
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
    ui->setupUi(this);
    resize(size() * Settings::uiScale);
    QPushButton *buttons[] = {ui->playButton, ui->pauseButton, ui->stopButton, ui->volumeButton, ui->netButton};
    for (int i = 0; i < 5; i++)
        buttons[i]->setIconSize(QSize(24, 24) * Settings::uiScale);

    // Add player_core frame
    player_core = new PlayerCore;
    ui->playerLayout->addWidget(player_core, 1);
    // Enable autohiding playlist
    player_core->installEventFilter(this);
    player_core->setMouseTracking(true);
    if (player_core->getLayer())
    {
        player_core->getLayer()->installEventFilter(this);
        player_core->getLayer()->setMouseTracking(true);
    }

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

    connect(player_core, &PlayerCore::paused,  ui->playButton,  &QPushButton::show);
    connect(player_core, &PlayerCore::paused,  ui->pauseButton, &QPushButton::hide);
    connect(player_core, &PlayerCore::played,  ui->playButton,  &QPushButton::hide);
    connect(player_core, &PlayerCore::played,  ui->pauseButton, &QPushButton::show);
#ifdef Q_OS_MAC
    connect(player_core, &PlayerCore::stopped, this, &ClassicPlayer::onStopped, Qt::QueuedConnection);
#else
    connect(player_core, &PlayerCore::stopped,       this, &ClassicPlayer::onStopped);
#endif
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

    connect(playlist, &Playlist::fileSelected, player_core, &PlayerCore::openFile);

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
    player_core->stop();
    webvideo->close();
    transformer->close();
    e->accept();
}

bool ClassicPlayer::eventFilter(QObject *obj, QEvent *e)
{
    static bool ctrl_pressed = false;

    // Hide/show playlist and toolbar
    if (e->type() == QEvent::Enter && obj == player_core && isFullScreen())
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
            player_core->screenShot();
            return true;
        case Qt::Key_C:
            showCutterbar();
            return true;
        case Qt::Key_Space:
            player_core->changeState();
            return true;
        case Qt::Key_Return:
            setFullScreen();
            return true;
        case Qt::Key_R:
            player_core->speedSetToDefault();
            return true;
        case Qt::Key_Left:
            if (ctrl_pressed)
                player_core->speedDown();
            else
                ui->progressBar->setValue(ui->progressBar->value() - 5);
            return true;

        case Qt::Key_Right:
            if (ctrl_pressed)
                player_core->speedUp();
            else
                ui->progressBar->setValue(ui->progressBar->value() + 5);
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

void ClassicPlayer::onSizeChanged(const QSize &sz)
{
    if (isFullScreen() || isMaximized())
        return;
    if (!Settings::autoResize)
        return;
    QSize newsize(sz.width(), height() - player_core->height() + sz.height());
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
    ui->statusBar->showMessage(time);
}

void ClassicPlayer::onPBarChanged(int pos)
{
    if (player_core->state == PlayerCore::STOPPING)
        return;
    if (ui->progressBar->isSliderDown())
        ui->statusBar->showMessage(secToTime(ui->progressBar->value()));
    else
        player_core->setProgress(pos);
}

void ClassicPlayer::onPBarReleased()
{
    if (player_core->state == PlayerCore::STOPPING)
        return;
    player_core->setProgress(ui->progressBar->value());
    ui->statusBar->clearMessage();
}

void ClassicPlayer::onProgressChanged(int pos)
{
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
    static QUrl url("https://github.com/coslyk/moonplayer/wiki/BroswerExtensionZH");
    QDesktopServices::openUrl(url);
}
