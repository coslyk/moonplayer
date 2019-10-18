#include "modernwindow.h"
#include "ui_modernwindow.h"
#include "playercore.h"
#include "playlist.h"
#include "reslibrary.h"
#include "settingsdialog.h"
#include "skin.h"
#include "utils.h"
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QResizeEvent>
#include <QTimer>

ModernWindow::ModernWindow(QWidget *parent) :
    WindowBase(parent),
    ui(new Ui::ModernWindow)
{
    // init ui
    setWindowFlags(Qt::FramelessWindowHint);
    ui->setupUi(this);

    ui->pauseButton->hide();
    QPushButton *buttons[] = {ui->playButton, ui->pauseButton, ui->stopButton};
    for (int i = 0; i < 3; i++)
    {
        buttons[i]->setIconSize(QSize(16, 16));
        buttons[i]->setFixedSize(QSize(32, 32));
    }
    QPushButton *buttons2[] = {ui->playlistButton, ui->searchButton, ui->volumeButton, ui->settingsButton};
    for (int i = 0; i < 4; i++)
    {
        buttons2[i]->setIconSize(QSize(16, 16));
        buttons2[i]->setFixedSize(QSize(24, 20));
    }
    ui->controllerWidget->setFixedSize(QSize(450, 70));
    ui->closeButton->setFixedSize(QSize(16, 16));
    ui->minButton->setFixedSize(QSize(16, 16));
    ui->maxButton->setFixedSize(QSize(16, 16));
    ui->titleBar->setFixedHeight(24);
    ui->titlebarLayout->setContentsMargins(QMargins(8, 4, 8, 4));
    ui->titlebarLayout->setSpacing(4);
    leftBorder = new Border(this, Border::LEFT);
    rightBorder = new Border(this, Border::RIGHT);
    bottomBorder = new Border(this, Border::BOTTOM);
    bottomLeftBorder = new Border(this, Border::BOTTOMLEFT);
    bottomRightBorder = new Border(this, Border::BOTTOMRIGHT);
    QGridLayout *gridLayout = new QGridLayout(centralWidget());
    gridLayout->addWidget(ui->titleBar, 0, 0, 1, 3);
    gridLayout->addWidget(leftBorder, 1, 0, 1, 1);
    gridLayout->addWidget(rightBorder, 1, 2, 1, 1);
    gridLayout->addWidget(bottomBorder, 2, 1, 1, 1);
    gridLayout->addWidget(bottomLeftBorder, 2, 0, 1, 1);
    gridLayout->addWidget(bottomRightBorder, 2, 2, 1, 1);
    gridLayout->setMargin(0);
    gridLayout->setSpacing(0);
    setAcceptDrops(true);
    setMinimumSize(QSize(640, 360));

    core->setParent(centralWidget());
    centralWidget()->installEventFilter(this);
    centralWidget()->setMouseTracking(true);

    // create timer
    hideTimer = new QTimer(this);
    hideTimer->setSingleShot(true);

    connect(ui->playlistButton, &QPushButton::clicked, this, &ModernWindow::showPlaylist);
    connect(ui->stopButton, &QPushButton::clicked, this, &ModernWindow::onStopButton);
    connect(ui->maxButton, &QPushButton::clicked, this, &ModernWindow::onMaxButton);
    connect(ui->playButton, &QPushButton::clicked, core, &PlayerCore::changeState);
    connect(ui->pauseButton, &QPushButton::clicked, core, &PlayerCore::changeState);
    connect(ui->volumeButton, &QPushButton::clicked, this, &ModernWindow::showVolumeSlider);
    connect(ui->settingsButton, &QPushButton::clicked, settingsDialog, &SettingsDialog::exec);
    connect(ui->searchButton, &QPushButton::clicked, reslibrary, &ResLibrary::show);
    connect(ui->timeSlider, &QSlider::sliderPressed, this, &ModernWindow::onTimeSliderPressed);
    connect(ui->timeSlider, &QSlider::valueChanged, this, &ModernWindow::onTimeSliderValueChanged);
    connect(ui->timeSlider, &QSlider::sliderReleased, this, &ModernWindow::onTimeSliderReleased);

    connect(core, &PlayerCore::played, ui->pauseButton, &QPushButton::show);
    connect(core, &PlayerCore::played, ui->playButton, &QPushButton::hide);
    connect(core, &PlayerCore::paused, ui->playButton, &QPushButton::show);
    connect(core, &PlayerCore::paused, ui->pauseButton, &QPushButton::hide);
    connect(core, &PlayerCore::stopped, ui->playButton, &QPushButton::show);
    connect(core, &PlayerCore::stopped, ui->pauseButton, &QPushButton::hide);
    connect(core, &PlayerCore::sizeChanged, this, &ModernWindow::onSizeChanged);
    connect(core, &PlayerCore::lengthChanged, this, &ModernWindow::onLengthChanged);
    connect(core, &PlayerCore::timeChanged, this, &ModernWindow::onTimeChanged);

    connect(hideTimer, &QTimer::timeout, this, &ModernWindow::hideElements);
}

ModernWindow::~ModernWindow()
{
    delete ui;
}


void ModernWindow::hideElements()
{
    if (!ui->titleBar->geometry().contains(mapFromGlobal(QCursor::pos())))
        // mouse is not in titlebar
        ui->titleBar->hide();

    if (!ui->controllerWidget->geometry().contains(mapFromGlobal(QCursor::pos())))
        // mouse is not in controller
        ui->controllerWidget->hide();

    if (ui->titleBar->isHidden() && ui->controllerWidget->isHidden())
        setCursor(QCursor(Qt::BlankCursor));
}

void ModernWindow::onLengthChanged(int len)
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


void ModernWindow::onTimeChanged(int time)
{
    if (!ui->timeSlider->isSliderDown()) // Make slider easier to drag
    {
        ui->timeLabel->setText(secToTime(time));
        ui->timeSlider->setValue(time);
    }
}

void ModernWindow::onTimeSliderPressed()
{
    if (core->state == PlayerCore::STOPPING)
        return;
    QString time = secToTime(ui->timeSlider->value());
    ui->timeLabel->setText(time);
}

void ModernWindow::onTimeSliderValueChanged(int time)
{
    if (core->state == PlayerCore::STOPPING)
        return;
    if (ui->timeSlider->isSliderDown()) // move by mouse
        ui->timeLabel->setText(secToTime(time));
    else // move by keyboard
        core->seek(time);
}

void ModernWindow::onTimeSliderReleased()
{
    if (core->state == PlayerCore::STOPPING)
        return;
    core->seek(ui->timeSlider->value());
}

// resize
void ModernWindow::onSizeChanged(const QSize &sz)
{
    if (isFullScreen())
        return;
    QRect available = QApplication::desktop()->availableGeometry(this);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 6, 0))
    if (sz.width() / devicePixelRatioF() > available.width() || sz.height()/ devicePixelRatioF() > available.height())
        setGeometry(available);
    else
        resize(sz / devicePixelRatioF());
#else
    if (sz.width() > available.width() || sz.height() > available.height())
        setGeometry(available);
    else
        resize(sz);
#endif
}

// resize ui
void ModernWindow::resizeEvent(QResizeEvent *e)
{
    // resize player core
    core->resize(e->size());

    // move and resize controller
    int c_x = (e->size().width() - ui->controllerWidget->width()) / 2;
    int c_y = e->size().height() - 130;
    ui->controllerWidget->move(c_x, c_y);
    ui->controllerWidget->raise();

    // raise borders and titlebar
    leftBorder->raise();
    rightBorder->raise();
    bottomBorder->raise();
    bottomLeftBorder->raise();
    bottomRightBorder->raise();
    ui->titleBar->raise();

    e->accept();
}

// Move Window
bool ModernWindow::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);

    if (event->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
        if (e->button() == Qt::LeftButton)
            dPos = e->pos();
        return true;
    }
    else if (event->type() == QEvent::MouseMove)
    {
        QMouseEvent *e = static_cast<QMouseEvent*>(event);
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
        return true;
    }
    else if (event->type() == QEvent::MouseButtonRelease)
    {
        dPos = QPoint();
        return true;
    }
    return false;
}


// show playlist
void ModernWindow::showPlaylist()
{
    QPoint vbPos = ui->controllerWidget->mapToGlobal(ui->playlistButton->pos());
    playlist->move(vbPos.x(), vbPos.y() - playlist->height());
    playlist->show();
}

// show volume slider
void ModernWindow::showVolumeSlider()
{
    QWidget *volumePopup = volumeSlider->window();
    QPoint vbPos = ui->controllerWidget->mapToGlobal(ui->volumeButton->pos());
    volumePopup->move(vbPos.x(), vbPos.y() - volumePopup->height());
    volumePopup->show();
}

// enter / exit fullscreen
void ModernWindow::mouseDoubleClickEvent(QMouseEvent *e)
{
    /* On macOS, this event will be emitted without double-click when mouse
     * is moved to screen edge.
     * Is it a Qt's bug?
     */
    if (e->buttons() == Qt::LeftButton && QRect(0, 0, width(), height()).contains(e->pos(), true))
        setFullScreen();
    e->accept();
}

void ModernWindow::setFullScreen()
{
    // avoid freezing
    core->pauseRendering();
    QTimer::singleShot(1500, core, SLOT(unpauseRendering()));

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

// show / hide border when entering or exiting fullscreen
void ModernWindow::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::WindowStateChange)
    {
        QWindowStateChangeEvent *ce = static_cast<QWindowStateChangeEvent*>(e);
        if (isFullScreen())
        {
            leftBorder->setEnabled(false);
            rightBorder->setEnabled(false);
            bottomBorder->setEnabled(false);
            bottomLeftBorder->setEnabled(false);
            bottomRightBorder->setEnabled(false);
        }
        else
        {
            leftBorder->setEnabled(true);
            rightBorder->setEnabled(true);
            bottomBorder->setEnabled(true);
            bottomLeftBorder->setEnabled(true);
            bottomRightBorder->setEnabled(true);
        }
#ifdef Q_OS_MAC
        if ((ce->oldState() & Qt::WindowFullScreen) && !isFullScreen())
            setWindowFlag(Qt::FramelessWindowHint, true);
#endif
        ce->accept();
        return;
    }
    QWidget::changeEvent(e);
}

// enter or exit fullscreen by key press
void ModernWindow::keyPressEvent(QKeyEvent *e)
{
    switch (e->key()) {
    case Qt::Key_Return:
        setFullScreen();
        break;
    case Qt::Key_Escape:
        if (isFullScreen())
            setFullScreen();
        break;
    default:
        WindowBase::keyPressEvent(e);
        return;
    }
    e->accept();
}


// maximize or normalize window
void ModernWindow::onMaxButton()
{
    if (isFullScreen())
        return;
    else if (isMaximized())
        showNormal();
    else
        showMaximized();
}
