#include "classicwindow.h"
#include "ui_classicwindow.h"
#include "playercore.h"
#include "playlist.h"
#include "reslibrary.h"
#include "settingsdialog.h"
#include "utils.h"
#include "windowbase.h"
#include <QDesktopWidget>
#include <QMouseEvent>
#include <QTimer>

ClassicWindow::ClassicWindow(QWidget *parent) :
    WindowBase(parent),
    ui(new Ui::ClassicWindow)
{
    ui->setupUi(this);
    ui->pauseButton->hide();
    ui->playerLayout->addWidget(core);

    connect(ui->playlistButton, &QPushButton::clicked, this, &ClassicWindow::showPlaylist);
    connect(ui->stopButton, &QPushButton::clicked, this, &ClassicWindow::onStopButton);
    connect(ui->playButton, &QPushButton::clicked, core, &PlayerCore::changeState);
    connect(ui->pauseButton, &QPushButton::clicked, core, &PlayerCore::changeState);
    connect(ui->volumeButton, &QPushButton::clicked, this, &ClassicWindow::showVolumeSlider);
    connect(ui->netButton, &QPushButton::clicked, res_library, &ResLibrary::show);
    connect(ui->timeSlider, &QSlider::sliderPressed, this, &ClassicWindow::onTimeSliderPressed);
    connect(ui->timeSlider, &QSlider::valueChanged, this, &ClassicWindow::onTimeSliderValueChanged);
    connect(ui->timeSlider, &QSlider::sliderReleased, this, &ClassicWindow::onTimeSliderReleased);

    /*connect(ui->brightnessSlider, &QSlider::valueChanged, core, &PlayerCore::setBrightness);
    connect(ui->contrastSlider, &QSlider::valueChanged, core, &PlayerCore::setContrast);
    connect(ui->saturationSlider, &QSlider::valueChanged, core, &PlayerCore::setSaturation);
    connect(ui->gammaSlider, &QSlider::valueChanged, core, &PlayerCore::setGamma);
    connect(ui->hueSlider, &QSlider::valueChanged, core, &PlayerCore::setHue);*/

    connect(core, &PlayerCore::played, ui->pauseButton, &QPushButton::show);
    connect(core, &PlayerCore::played, ui->playButton, &QPushButton::hide);
    connect(core, &PlayerCore::paused, ui->playButton, &QPushButton::show);
    connect(core, &PlayerCore::paused, ui->pauseButton, &QPushButton::hide);
    connect(core, &PlayerCore::stopped, ui->playButton, &QPushButton::show);
    connect(core, &PlayerCore::stopped, ui->pauseButton, &QPushButton::hide);
    connect(core, &PlayerCore::sizeChanged, this, &ClassicWindow::onSizeChanged);
    connect(core, &PlayerCore::lengthChanged, this, &ClassicWindow::onLengthChanged);
    connect(core, &PlayerCore::timeChanged, this, &ClassicWindow::onTimeChanged);
}

ClassicWindow::~ClassicWindow()
{
    delete ui;
}

// Resize mpv window
void ClassicWindow::onSizeChanged(const QSize &video_size)
{
    if (isFullScreen())
        return;
    QSize sz = video_size + size() - core->size();
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

void ClassicWindow::onLengthChanged(int len)
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

void ClassicWindow::onTimeChanged(int time)
{
    if (!ui->timeSlider->isSliderDown()) // Make slider easier to drag
    {
        ui->timeLabel->setText(secToTime(time));
        ui->timeSlider->setValue(time);
    }
}

void ClassicWindow::onTimeSliderPressed()
{
    if (core->state == PlayerCore::STOPPING)
        return;
    QString time = secToTime(ui->timeSlider->value());
    ui->timeLabel->setText(time);
}

void ClassicWindow::onTimeSliderValueChanged(int time)
{
    if (core->state == PlayerCore::STOPPING)
        return;
    if (ui->timeSlider->isSliderDown()) // move by mouse
        ui->timeLabel->setText(secToTime(time));
    else // move by keyboard
        core->seek(time);
}

void ClassicWindow::onTimeSliderReleased()
{
    if (core->state == PlayerCore::STOPPING)
        return;
    core->seek(ui->timeSlider->value());
}

// show playlist
void ClassicWindow::showPlaylist()
{
    QPoint vbPos = ui->toolbar->mapToGlobal(ui->playlistButton->pos());
    playlist->move(vbPos.x() - playlist->width() + ui->playlistButton->width(), vbPos.y() - playlist->height());
    playlist->show();
}

// show volume slider
void ClassicWindow::showVolumeSlider()
{
    QWidget *volumePopup = volumeSlider->window();
    QPoint vbPos = ui->toolbar->mapToGlobal(ui->volumeButton->pos());
    volumePopup->move(vbPos.x(), vbPos.y() - volumePopup->height());
    volumePopup->show();
}


// enter / exit fullscreen
void ClassicWindow::mouseDoubleClickEvent(QMouseEvent *e)
{
    /* On macOS, this event will be emitted without double-click when mouse
     * is moved to screen edge.
     * Is it a Qt's bug?
     */
    if (e->buttons() == Qt::LeftButton && QRect(0, 0, width(), height()).contains(e->pos(), true))
        setFullScreen();
    e->accept();
}

void ClassicWindow::setFullScreen()
{
    // avoid freezing
    core->pauseRendering();
    QTimer::singleShot(1500, core, SLOT(unpauseRendering()));

    if (isFullScreen())
        showNormal();
    else
        showFullScreen();
}

// enter or exit fullscreen by key press
void ClassicWindow::keyPressEvent(QKeyEvent *e)
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
