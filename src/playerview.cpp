#include "playerview.h"
#include "ui_playerview.h"
#include "cutterbar.h"
#include "downloader.h"
#include "playlist.h"
#include "playercore.h"
#include "reslibrary.h"
#include "selectiondialog.h"
#include "settings_player.h"
#include "settings_audio.h"
#include "settingsdialog.h"
#include "skin.h"
#include "utils.h"
#include "yougetbridge.h"
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QResizeEvent>
#include <QTimer>

PlayerView::PlayerView(QWidget *parent) :
    QWidget(parent, Qt::FramelessWindowHint),
    ui(new Ui::PlayerView)
{
    // init ui
    ui->setupUi(this);
    ui->pauseButton->hide();
    QPushButton *buttons[] = {ui->playButton, ui->pauseButton, ui->stopButton};
    for (int i = 0; i < 3; i++)
    {
        buttons[i]->setIconSize(QSize(16, 16) * Settings::uiScale);
        buttons[i]->setFixedSize(QSize(32, 32) * Settings::uiScale);
    }
    QPushButton *buttons2[] = {ui->playlistButton, ui->searchButton, ui->volumeButton, ui->settingsButton, ui->hideEqualizerButton};
    for (int i = 0; i < 5; i++)
    {
        buttons2[i]->setIconSize(QSize(16, 16) * Settings::uiScale);
        buttons2[i]->setFixedSize(QSize(24, 20) * Settings::uiScale);
    }
    ui->controllerWidget->setFixedSize(QSize(450, 70) * Settings::uiScale);
    ui->equalizerWidget->setFixedSize(QSize(350, 180) * Settings::uiScale);
    ui->equalizerWidget->hide();
    ui->closeButton->setFixedSize(QSize(16, 16) * Settings::uiScale);
    ui->minButton->setFixedSize(QSize(16, 16) * Settings::uiScale);
    ui->maxButton->setFixedSize(QSize(16, 16) * Settings::uiScale);
    ui->titleBar->setFixedHeight(24 * Settings::uiScale);
    ui->titlebarLayout->setContentsMargins(QMargins(8, 4, 8, 4) * Settings::uiScale);
    ui->titlebarLayout->setSpacing(4 * Settings::uiScale);
    leftBorder = new Border(this, Border::LEFT);
    rightBorder = new Border(this, Border::RIGHT);
    bottomBorder = new Border(this, Border::BOTTOM);
    QGridLayout *gridLayout = new QGridLayout(this);
    gridLayout->addWidget(ui->titleBar, 0, 0, 1, 3);
    gridLayout->addWidget(leftBorder, 1, 0, 1, 1);
    gridLayout->addWidget(rightBorder, 1, 2, 1, 1);
    gridLayout->addWidget(bottomBorder, 2, 1, 1, 1);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);
    setAcceptDrops(true);

    quit_requested = false;
    no_play_next = false;
    ctrl_pressed = false;

    // create player core
    core = new PlayerCore(this);
    core->move(0, 0);
    core->setAttribute(Qt::WA_TransparentForMouseEvents);

    // create settings dialog
    settingsDialog = new SettingsDialog(this);

    // create selection dialo
    selectionDialog = new SelectionDialog(this);

    // create playlist
    playlist = new Playlist(this);
    playlist->setWindowFlags(playlist->windowFlags() | Qt::Popup);
    playlist->setFixedSize(QSize(200, 350) * Settings::uiScale);

    // create library viewer
    reslibrary = new ResLibrary;

    // create volume slider
    QWidget *volumePopup = new QWidget(this, Qt::Popup);
    volumePopup->resize(QSize(24, 80) * Settings::uiScale);
    volumeSlider = new QSlider(Qt::Vertical, volumePopup);
    volumeSlider->setRange(0, 10);
    volumeSlider->setValue(10);
    volumeSlider->resize(QSize(20, 70) * Settings::uiScale);
    volumeSlider->move(2, 5);

    // create menu
    QMenu *video_menu = new QMenu(tr("Video"));
    video_menu->addAction("4:3", core, SLOT(setRatio_4_3()));
    video_menu->addAction("16:9", core, SLOT(setRatio_16_9()));
    video_menu->addAction("16:10", core, SLOT(setRatio_16_10()));
    video_menu->addAction(tr("Default"), core, SLOT(setRatio_0()));
    video_menu->addSeparator();
    video_menu->addAction(tr("Equalizer"), ui->equalizerWidget, SLOT(show()));

    QMenu *audio_menu = new QMenu(tr("Audio"));
    audio_menu->addAction(tr("Stereo"), core, SLOT(setChannel_Stereo()));
    audio_menu->addAction(tr("Left channel"), core, SLOT(setChannel_Left()));
    audio_menu->addAction(tr("Right channel"), core, SLOT(setChannel_Right()));
    audio_menu->addAction(tr("Swap channel"), core, SLOT(setChannel_Swap()));
    audio_menu->addSeparator();
    audio_menu->addAction(tr("Select track"), this, SLOT(selectAudioTrack()));

    QMenu *speed_menu = new QMenu(tr("Speed"));
    speed_menu->addAction(tr("Speed up"), core, SLOT(speedUp()), QKeySequence("Ctrl+Right"));
    speed_menu->addAction(tr("Speed down"), core, SLOT(speedDown()), QKeySequence("Ctrl+Left"));
    speed_menu->addAction(tr("Default"), core, SLOT(speedSetToDefault()), QKeySequence("R"));

    QMenu *sub_menu = new QMenu(tr("Subtitle"));
    sub_menu->addAction(tr("Visible"), core, SLOT(switchDanmaku()), QKeySequence("D"));
    sub_menu->addAction(tr("Select"), this, SLOT(selectSubtitle()));
    sub_menu->addAction(tr("Load from file"), this, SLOT(addSubtitle()));

    menu = new QMenu(this);
    menu->addMenu(video_menu);
    menu->addMenu(audio_menu);
    menu->addMenu(sub_menu);
    menu->addMenu(speed_menu);

    menu->addSeparator();
    menu->addAction(tr("Screenshot"), core, SLOT(screenShot()), QKeySequence("S"));
    menu->addAction(tr("Cut video"), this, SLOT(showCutterBar()), QKeySequence("C"));

    menu->addSeparator();
    menu->addAction(tr("Online video"), reslibrary, SLOT(show()), QKeySequence("W"));
    menu->addAction(tr("Settings"), settingsDialog, SLOT(show()), QKeySequence("Ctrl+,"));
    menu->addAction(tr("Update you-get"), &you_get_bridge, SLOT(updateYouGet()));
    menu->addAction(tr("Ext. for browser"), this, SLOT(openExtPage()));
    QMenu *aboutMenu = menu->addMenu(tr("About"));
    aboutMenu->addAction(tr("Contribute"), this, SLOT(openContributePage()));
    aboutMenu->addAction(tr("Homepage"), this, SLOT(openHomepage()));

    // create cutterbar
    cutterBar = new CutterBar(this);
    cutterBar->setWindowFlags(cutterBar->windowFlags() | Qt::Popup);

    // create timer
    hideTimer = new QTimer(this);
    hideTimer->setSingleShot(true);
    setMouseTracking(true);

    connect(core, &PlayerCore::lengthChanged, this, &PlayerView::onLengthChanged);
    connect(core, &PlayerCore::sizeChanged, this, &PlayerView::onSizeChanged);
    connect(core, &PlayerCore::timeChanged, this, &PlayerView::onTimeChanged);
    connect(core, &PlayerCore::played, ui->pauseButton, &QPushButton::show);
    connect(core, &PlayerCore::played, ui->playButton, &QPushButton::hide);
    connect(core, &PlayerCore::paused, ui->playButton, &QPushButton::show);
    connect(core, &PlayerCore::paused, ui->pauseButton, &QPushButton::hide);
    connect(core, &PlayerCore::stopped, this, &PlayerView::onStopped);
    connect(playlist, &Playlist::fileSelected, core, &PlayerCore::openFile);
    connect(hideTimer, &QTimer::timeout, this, &PlayerView::hideElements);
    connect(volumeSlider, &QSlider::valueChanged, core, &PlayerCore::setVolume);
    connect(volumeSlider, &QSlider::valueChanged, this, &PlayerView::saveVolume);
    connect(cutterBar, &CutterBar::newFrame, core, &PlayerCore::jumpTo);
    connect(ui->playlistButton, &QPushButton::clicked, this, &PlayerView::showPlaylist);
    connect(ui->stopButton, &QPushButton::clicked, this, &PlayerView::onStopButton);
    connect(ui->maxButton, &QPushButton::clicked, this, &PlayerView::onMaxButton);
    connect(ui->playButton, &QPushButton::clicked, core, &PlayerCore::changeState);
    connect(ui->pauseButton, &QPushButton::clicked, core, &PlayerCore::changeState);
    connect(ui->volumeButton, &QPushButton::clicked, this, &PlayerView::showVolumeSlider);
    connect(ui->settingsButton, &QPushButton::clicked, settingsDialog, &SettingsDialog::exec);
    connect(ui->searchButton, &QPushButton::clicked, reslibrary, &ResLibrary::show);
    connect(ui->timeSlider, &QSlider::sliderPressed, this, &PlayerView::onTimeSliderPressed);
    connect(ui->timeSlider, &QSlider::valueChanged, this, &PlayerView::onTimeSliderValueChanged);
    connect(ui->timeSlider, &QSlider::sliderReleased, this, &PlayerView::onTimeSliderReleased);

    connect(ui->brightnessSlider, &QSlider::valueChanged, core, &PlayerCore::setBrightness);
    connect(ui->contrastSlider, &QSlider::valueChanged, core, &PlayerCore::setContrast);
    connect(ui->saturationSlider, &QSlider::valueChanged, core, &PlayerCore::setSaturation);
    connect(ui->gammaSlider, &QSlider::valueChanged, core, &PlayerCore::setGamma);
    connect(ui->hueSlider, &QSlider::valueChanged, core, &PlayerCore::setHue);

    volumeSlider->setValue(Settings::volume);
}

PlayerView::~PlayerView()
{
    delete ui;
}

void PlayerView::onStopButton()
{
    no_play_next = true;
    core->stop();
}

void PlayerView::onStopped()
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

void PlayerView::closeEvent(QCloseEvent *e)
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

        reslibrary->close();
        no_play_next = true;

    // It's not safe to quit until mpv is stopped
    if (core->state != PlayerCore::STOPPING)
    {
        core->stop();
        quit_requested = true;
        e->ignore();
    }
    else
        e->accept();
}

// Drag & drop files
void PlayerView::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void PlayerView::dropEvent(QDropEvent *e)
{
    QList<QUrl> urls = e->mimeData()->urls();
    bool first = true;
    foreach (QUrl url, urls) {
        if (url.isLocalFile())
        {
            QString file = url.toLocalFile();
            // subtitle
            if ((file.endsWith(".srt") || file.endsWith(".ass")) && core->state != PlayerCore::STOPPING)
                core->openSubtitle(file);
            // first file
            else if (first)
            {
                playlist->addFileAndPlay(QFileInfo(file).fileName(), file);
                first = false;
            }
            // not first file
            else
                playlist->addFile(QFileInfo(file).fileName(), file);
        }
        else if (!url.scheme().isEmpty())
            playlist->addUrl(url.toString());
    }
    e->accept();
}

// resize ui
void PlayerView::resizeEvent(QResizeEvent *e)
{
    // resize player core
    core->resize(e->size());

    // move and resize controller
    int c_x = (e->size().width() - ui->controllerWidget->width()) / 2;
    int c_y = e->size().height() - 130;
    ui->controllerWidget->move(c_x, c_y);
    ui->controllerWidget->raise();

    // move and resize equalizer
    int e_x = (e->size().width() - ui->equalizerWidget->width()) / 2;
    int e_y = (e->size().height() - ui->equalizerWidget->height()) / 2;
    ui->equalizerWidget->move(e_x, e_y);
    ui->equalizerWidget->raise();

    // raise borders and titlebar
    leftBorder->raise();
    rightBorder->raise();
    bottomBorder->raise();
    ui->titleBar->raise();\

    e->accept();
}

// Keyboard shortcuts
void PlayerView::keyPressEvent(QKeyEvent *e)
{
    switch (e->key())
    {
    case Qt::Key_Control:
        ctrl_pressed = true;
        break;
    case Qt::Key_S:
        core->screenShot();
        break;
    case Qt::Key_C:
        showCutterBar();
        break;
    case Qt::Key_D:
        core->switchDanmaku();
        break;
    case Qt::Key_L:
        showPlaylist();
        break;
    case Qt::Key_O:
        if (ctrl_pressed)
            playlist->onAddItem();
        break;
    case Qt::Key_U:
        if (ctrl_pressed)
            playlist->onNetItem();
        break;
    case Qt::Key_W:
        reslibrary->show();
        break;
    case Qt::Key_Space:
        core->changeState();
        break;
    case Qt::Key_Return:
        setFullScreen();
        break;
    case Qt::Key_R:
        core->speedSetToDefault();
        break;
    case Qt::Key_Comma:
        if (ctrl_pressed)
            settingsDialog->exec();
        break;
    case Qt::Key_Left:
        if (ctrl_pressed)
            core->speedDown();
        else
            ui->timeSlider->setValue(ui->timeSlider->value() - 5);
        break;

    case Qt::Key_Right:
        if (ctrl_pressed)
            core->speedUp();
        else
            ui->timeSlider->setValue(ui->timeSlider->value() + 5);
        break;

    case Qt::Key_Up:
        volumeSlider->setValue(volumeSlider->value() + 1);
        break;
    case Qt::Key_Down:
        volumeSlider->setValue(volumeSlider->value() - 1);
        break;
    default: break;
    }
    e->accept();
}

void PlayerView::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Control)
        ctrl_pressed = false;
    e->accept();
}

void PlayerView::mouseDoubleClickEvent(QMouseEvent *e)
{
    /* On macOS, this event will be emitted without double-click when mouse
     * is moved to screen edge.
     * Is it a Qt's bug?
     */
    if (e->buttons() == Qt::LeftButton && QRect(0, 0, width(), height()).contains(e->pos(), true))
        setFullScreen();
    e->accept();
}


void PlayerView::mousePressEvent(QMouseEvent *e)
{
    if (!isMaximized() && e->button() == Qt::LeftButton)
        dPos = e->pos();
    e->accept();
}

void PlayerView::mouseMoveEvent(QMouseEvent *e)
{
    // move window
    if (!dPos.isNull())
        move(e->globalPos() - dPos);

    // show controller, titlebar and cursor
    hideTimer->stop();
    ui->controllerWidget->show();
    ui->titleBar->show();
    setCursor(QCursor(Qt::ArrowCursor));
    if (core->state == PlayerCore::VIDEO_PLAYING || core->state == PlayerCore::TV_PLAYING)
        hideTimer->start(2000);
    e->accept();
}

void PlayerView::mouseReleaseEvent(QMouseEvent *e)
{
    dPos = QPoint();
    e->accept();
}

void PlayerView::contextMenuEvent(QContextMenuEvent *e)
{
    menu->exec(QCursor::pos());
    e->accept();
}

void PlayerView::hideElements()
{
    ui->controllerWidget->hide();
    ui->titleBar->hide();
    if (!ui->equalizerWidget->isVisible())
        setCursor(QCursor(Qt::BlankCursor));
}

void PlayerView::onLengthChanged(int len)
{
    if (len == 0) //playing TV
        ui->timeSlider->setEnabled(false);
    else //playing video
    {
        ui->timeSlider->setEnabled(true);
        ui->timeSlider->setMaximum(len);
        ui->durationLabel->setText(secToTime(len));
    }
    activateWindow();
    raise();
}

void PlayerView::onTimeChanged(int time)
{
    ui->timeLabel->setText(secToTime(time));
    if (!ui->timeSlider->isSliderDown() && time % 4 == 0) // Make slider easier to drag
        ui->timeSlider->setValue(time);
}

void PlayerView::onTimeSliderPressed()
{
    if (core->state == PlayerCore::STOPPING)
        return;
    QString time = secToTime(ui->timeSlider->value());
    ui->timeLabel->setText(time);
}

void PlayerView::onTimeSliderValueChanged(int time)
{
    if (core->state == PlayerCore::STOPPING)
        return;
    if (ui->timeSlider->isSliderDown()) // move by mouse
        ui->timeLabel->setText(secToTime(time));
    else // move by keyboard
        core->setProgress(time);
}

void PlayerView::onTimeSliderReleased()
{
    if (core->state == PlayerCore::STOPPING)
        return;
    core->setProgress(ui->timeSlider->value());
}

void PlayerView::onSizeChanged(const QSize &sz)
{
    if (isFullScreen() || isMaximized())
        return;
    QRect available = QApplication::desktop()->availableGeometry(this);
    if (sz.width() > available.width() || sz.height() > available.height())
        setGeometry(available);
    else
        resize(sz);
}

// show cutterbar
void PlayerView::showCutterBar()
{
    if (core->state == PlayerCore::STOPPING || core->state == PlayerCore::TV_PLAYING || cutterBar->isVisible())
        return;
    QString filename = core->currentFile();
    if (filename.startsWith("http"))
    {
        QMessageBox::warning(this, "Error", tr("Only support cutting local videos!"));
        return;
    }
    if (core->state == PlayerCore::VIDEO_PLAYING) //pause
        core->changeState();
    cutterBar->init(filename, core->getLength(), core->getTime());
    cutterBar->move(mapToGlobal(QPoint(50, 50)));
    cutterBar->show();
}

// show playlist
void PlayerView::showPlaylist()
{
    QPoint vbPos = ui->controllerWidget->mapToGlobal(ui->playlistButton->pos());
    playlist->move(vbPos.x(), vbPos.y() - playlist->height());
    playlist->show();
}

// show volume slider
void PlayerView::showVolumeSlider()
{
    QWidget *volumePopup = volumeSlider->window();
    QPoint vbPos = ui->controllerWidget->mapToGlobal(ui->volumeButton->pos());
    volumePopup->move(vbPos.x(), vbPos.y() - volumePopup->height());
    volumePopup->show();
}

void PlayerView::saveVolume(int vol)
{
    Settings::volume = vol;
}

// show or exit fullscreen
void PlayerView::setFullScreen()
{
    // avoid freezing
    core->pauseRendering();
    QTimer::singleShot(1000, core, SLOT(unpauseRendering()));

    if (isFullScreen())
        showNormal();
    else
    {
#ifdef Q_OS_MAC
        setWindowFlag(Qt::FramelessWindowHint, false);
        show();
        QTimer::singleShot(0, this, SLOT(showFullScreen()));
#else
        showFullScreen();
#endif
    }
}

#ifdef Q_OS_MAC
void PlayerView::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent *ce = static_cast<QWindowStateChangeEvent*>(e);
        if ((ce->oldState() & Qt::WindowFullScreen) && !isFullScreen())
            setWindowFlag(Qt::FramelessWindowHint, true);
        ce->accept();
        return;
    }
    QWidget::changeEvent(e);
}
#endif


// add & select subtitle
void PlayerView::addSubtitle()
{
    QString videoFile = core->currentFile();
    QString dir = videoFile.startsWith('/') ? QFileInfo(videoFile).path() : QDir::homePath();
    QString subFile = QFileDialog::getOpenFileName(this, tr("Open subtitle file"), dir);
    if (!subFile.isEmpty())
        core->openSubtitle(subFile);
}

void PlayerView::selectSubtitle()
{
    int sid = selectionDialog->showDialog_Index(core->getSubtitleList(), tr("Select subtitle:"));
    if (sid != -1)
        core->setSid(sid);
}

// select audio track
void PlayerView::selectAudioTrack()
{
    int aid = selectionDialog->showDialog_Index(core->getAudioTracksList(), tr("Select audio track:"));
    if (aid != -1)
        core->setAid(aid);
}


// maximize or normalize window
void PlayerView::onMaxButton()
{
    if (isFullScreen())
        return;
    else if (isMaximized())
        showNormal();
    else
        showMaximized();
}

//open homepage
void PlayerView::openHomepage()
{
    static QUrl url("https://github.com/coslyk/moonplayer");
    QDesktopServices::openUrl(url);
}

//open extension page
void PlayerView::openExtPage()
{
    static QUrl url("https://github.com/coslyk/moonplayer/wiki/BroswerExtension");
    QDesktopServices::openUrl(url);
}

//open contribute page
void PlayerView::openContributePage()
{
    static QUrl url("https://github.com/coslyk/moonplayer/wiki/Contribute");
    QDesktopServices::openUrl(url);
}