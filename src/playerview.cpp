#include "playerview.h"
#include "ui_playerview.h"
#include "playlist.h"
#include "playercore.h"
#include "settings_player.h"
#include "skin.h"
#include "utils.h"
#include <QDesktopWidget>
#include <QGridLayout>
#include <QResizeEvent>
#include <QTimer>

PlayerView::PlayerView(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PlayerView)
{
    // init ui
    ui->setupUi(this);
    ui->pauseButton->hide();
    setWindowFlag(Qt::FramelessWindowHint);
    QPushButton *buttons[] = {ui->playButton, ui->pauseButton, ui->stopButton};
    for (int i = 0; i < 3; i++)
    {
        buttons[i]->setIconSize(QSize(16, 16) * Settings::uiScale);
        buttons[i]->setFixedSize(QSize(32, 32) * Settings::uiScale);
    }
    QPushButton *buttons2[] = {ui->playlistButton, ui->searchButton, ui->volumeButton};
    for (int i = 0; i < 3; i++)
    {
        buttons2[i]->setIconSize(QSize(16, 16) * Settings::uiScale);
        buttons2[i]->setFixedSize(QSize(24, 20) * Settings::uiScale);
    }
    ui->controllerWidget->setFixedSize(QSize(450, 70) * Settings::uiScale);
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

    quit_requested = false;
    no_play_next = false;

    // create player core
    core = new PlayerCore(this);
    core->move(0, 0);
    core->setAttribute(Qt::WA_TransparentForMouseEvents);

    // create playlist
    playlist = new Playlist(this);
    playlist->hide();

    // create volume slider
    volumeSlider = new QSlider(Qt::Vertical, this);
    volumeSlider->setWindowFlag(Qt::Popup);
    volumeSlider->setRange(0, 10);
    volumeSlider->setValue(10);

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
    connect(ui->playlistButton, &QPushButton::clicked, this, &PlayerView::showHidePlaylist);
    connect(ui->stopButton, &QPushButton::clicked, this, &PlayerView::onStopButton);
    connect(ui->playButton, &QPushButton::clicked, core, &PlayerCore::changeState);
    connect(ui->pauseButton, &QPushButton::clicked, core, &PlayerCore::changeState);
    connect(ui->volumeButton, &QPushButton::clicked, this, &PlayerView::showVolumeSlider);
    connect(ui->timeSlider, &QSlider::valueChanged, core, &PlayerCore::setProgress);
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

void PlayerView::resizeEvent(QResizeEvent *e)
{
    // resize player core
    core->resize(e->size());

    // move and resize controller
    int c_x = (e->size().width() - ui->controllerWidget->width()) / 2;
    int c_y = e->size().height() - 130;
    ui->controllerWidget->move(c_x, c_y);
    ui->controllerWidget->raise();

    // move and resize playlist
    playlist->resize(playlist->width(), e->size().height() - ui->titleBar->height());
    playlist->move(e->size().width() - playlist->width(), ui->titleBar->height());
    playlist->raise();

    // raise borders and titlebar
    leftBorder->raise();
    rightBorder->raise();
    bottomBorder->raise();
    ui->titleBar->raise();

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

void PlayerView::hideElements()
{
    ui->controllerWidget->hide();
    ui->titleBar->hide();
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

void PlayerView::onSizeChanged(const QSize &sz)
{
    if (isFullScreen() || isMaximized())
        return;
    QRect available = QApplication::desktop()->availableGeometry();
    if (sz.width() > available.width() || sz.height() > available.height())
        setGeometry(available);
    else
        resize(sz);
}


void PlayerView::showHidePlaylist()
{
    playlist->setVisible(!playlist->isVisible());
}

void PlayerView::showVolumeSlider()
{
    QPoint vbPos = ui->controllerWidget->mapToGlobal(ui->volumeButton->pos());
    volumeSlider->move(vbPos.x(), vbPos.y() - volumeSlider->height());
    volumeSlider->show();
}
