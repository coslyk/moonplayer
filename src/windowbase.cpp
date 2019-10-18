#include "windowbase.h"
#include "aboutdialog.h"
#include "cutterbar.h"
#include "downloader.h"
#include "parserbase.h"
#include "playlist.h"
#include "playercore.h"
#include "reslibrary.h"
#include "selectiondialog.h"
#include "settings_audio.h"
#include "settingsdialog.h"
#include "upgraderdialog.h"
#include "ui_equalizer.h"
#include <QApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QInputDialog>
#include <QMenu>
#include <QMessageBox>
#include <QMimeData>
#include <QResizeEvent>
#include <QTimer>

WindowBase::WindowBase(QWidget *parent) :
    QMainWindow(parent)
{
    quit_requested = false;
    no_play_next = false;
    ctrl_pressed = false;

    // create player core
    core = new PlayerCore;
    core->setAttribute(Qt::WA_TransparentForMouseEvents);

    // create settings dialog
    settingsDialog = new SettingsDialog(this);

    // create selection dialog
    selectionDialog = new SelectionDialog(this);

    // create playlist
    playlist = new Playlist(this);
    playlist->setWindowFlags(playlist->windowFlags() | Qt::Popup);
    playlist->setFixedSize(QSize(200, 350));

    // create library viewer
    reslibrary = new ResLibrary;

    // create volume slider
    QWidget *volumePopup = new QWidget(this, Qt::Popup);
    volumePopup->resize(QSize(24, 80));
    volumeSlider = new QSlider(Qt::Vertical, volumePopup);
    volumeSlider->setRange(0, 10);
    volumeSlider->setValue(10);
    volumeSlider->resize(QSize(20, 70));
    volumeSlider->move(2, 5);

    // create upgrader dialog
    upgraderDialog = new UpgraderDialog(this);

    // create about dialog
    aboutDialog = new AboutDialog(this);

    // create equalizer
    equalizer = new QDialog(this);
    equalizer_ui = new Ui::Equalizer;
    equalizer_ui->setupUi(equalizer);

    // create menu
    QMenu *open_menu = new QMenu(tr("Open"));
    open_menu->addAction(tr("File") + "\tCtrl+O", playlist, SLOT(onAddItem()));
    open_menu->addAction(tr("Url") + "\tCtrl+U", playlist, SLOT(onNetItem()));
    open_menu->addAction(tr("Playlist"), playlist, SLOT(onListItem()));

    QMenu *video_menu = new QMenu(tr("Video"));
    video_menu->addAction("4:3", core, SLOT(setRatio_4_3()));
    video_menu->addAction("16:9", core, SLOT(setRatio_16_9()));
    video_menu->addAction("16:10", core, SLOT(setRatio_16_10()));
    video_menu->addAction(tr("Default"), core, SLOT(setRatio_0()));
    video_menu->addSeparator();
    video_menu->addAction(tr("Equalizer"), equalizer, SLOT(exec()));

    QMenu *audio_menu = new QMenu(tr("Audio"));
    audio_menu->addAction(tr("Stereo"), core, SLOT(setChannel_Stereo()));
    audio_menu->addAction(tr("Left channel"), core, SLOT(setChannel_Left()));
    audio_menu->addAction(tr("Right channel"), core, SLOT(setChannel_Right()));
    audio_menu->addAction(tr("Swap channel"), core, SLOT(setChannel_Swap()));
    audio_menu->addSeparator();
    audio_menu->addAction(tr("Select track"), this, SLOT(selectAudioTrack()));
    audio_menu->addAction(tr("Load from file"), this, SLOT(addAudioTrack()));
    audio_menu->addAction(tr("Delay"), this, SLOT(setAudioDelay()));

    QMenu *sub_menu = new QMenu(tr("Subtitle"));
    sub_menu->addAction(tr("Visible") + "\tD", core, SLOT(switchDanmaku()));
    sub_menu->addAction(tr("Select"), this, SLOT(selectSubtitle()));
    sub_menu->addAction(tr("Load from file"), this, SLOT(addSubtitle()));
    sub_menu->addAction(tr("Delay"), this, SLOT(setSubDelay()));

    QMenu *speed_menu = new QMenu(tr("Speed"));
    speed_menu->addAction(tr("Speed up") + "\tCtrl+Right", core, SLOT(speedUp()));
    speed_menu->addAction(tr("Speed down") + "\tCtrl+Left", core, SLOT(speedDown()));
    speed_menu->addAction(tr("Default") + "\tR", core, SLOT(speedSetToDefault()));

    menu = new QMenu(this);
    menu->addMenu(open_menu);
    menu->addAction(tr("Playlist") + "\tL", this, SLOT(showPlaylist()));
    menu->addAction(tr("Online video") + "\tW", reslibrary, SLOT(show()));
    menu->addSeparator();
    menu->addMenu(video_menu);
    menu->addMenu(audio_menu);
    menu->addMenu(sub_menu);
    menu->addMenu(speed_menu);

    menu->addSeparator();
    menu->addAction(tr("Screenshot") + "\tS", core, SLOT(screenShot()));
    menu->addAction(tr("Cut video") + "\tC", this, SLOT(showCutterBar()));

    menu->addSeparator();
    menu->addAction(tr("Settings") + "\tCtrl+,", settingsDialog, SLOT(exec()));
    menu->addAction(tr("Ext. for browser"), this, SLOT(openExtPage()));
    menu->addAction(tr("Upgrade parsers"), upgraderDialog, SLOT(runUpgrader()));
    menu->addAction(tr("About"), aboutDialog, SLOT(exec()));

    // create cutterbar
    cutterBar = new CutterBar(this);
    cutterBar->setWindowFlags(cutterBar->windowFlags() | Qt::Popup);

    connect(core, &PlayerCore::stopped, this, &WindowBase::onStopped);
    connect(downloader, SIGNAL(newFile(QString,QString)), playlist, SLOT(addFile(QString,QString)));
    connect(downloader, SIGNAL(newPlay(QString,QString)), playlist, SLOT(addFileAndPlay(QString,QString)));
    connect(playlist, &Playlist::fileSelected, core, &PlayerCore::openFile);
    connect(volumeSlider, &QSlider::valueChanged, core, &PlayerCore::setVolume);
    connect(volumeSlider, &QSlider::valueChanged, this, &WindowBase::saveVolume);
    connect(cutterBar, &CutterBar::newFrame, core, &PlayerCore::jumpTo);

    connect(equalizer_ui->brightnessSlider, &QSlider::valueChanged, core, &PlayerCore::setBrightness);
    connect(equalizer_ui->contrastSlider, &QSlider::valueChanged, core, &PlayerCore::setContrast);
    connect(equalizer_ui->saturationSlider, &QSlider::valueChanged, core, &PlayerCore::setSaturation);
    connect(equalizer_ui->gammaSlider, &QSlider::valueChanged, core, &PlayerCore::setGamma);
    connect(equalizer_ui->hueSlider, &QSlider::valueChanged, core, &PlayerCore::setHue);

    volumeSlider->setValue(Settings::volume);
}

WindowBase::~WindowBase()
{
    delete equalizer_ui;
}

void WindowBase::onStopButton()
{
    no_play_next = true;
    core->stop();
}

void WindowBase::onStopped()
{
    if (quit_requested)
    {
        quit_requested = false;
        close();
    }
    else if (no_play_next)
        no_play_next = false;
    else
        playlist->playNext();
}

void WindowBase::closeEvent(QCloseEvent *e)
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
void WindowBase::dragEnterEvent(QDragEnterEvent *e)
{
    if (e->mimeData()->hasUrls())
        e->acceptProposedAction();
}

void WindowBase::dropEvent(QDropEvent *e)
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

// Keyboard shortcuts
void WindowBase::keyPressEvent(QKeyEvent *e)
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
        {
            playlist->onAddItem();
            // key-release event may not be received after dialog is shown
            ctrl_pressed = false;
        }
        break;
    case Qt::Key_U:
        if (ctrl_pressed)
        {
            playlist->onNetItem();
            // key-release event may not be received after dialog is shown
            ctrl_pressed = false;
        }
        break;
    case Qt::Key_W:
        reslibrary->show();
        reslibrary->activateWindow();
        break;
    case Qt::Key_Space:
        core->changeState();
        break;
    case Qt::Key_R:
        core->speedSetToDefault();
        break;
    case Qt::Key_Comma:
        if (ctrl_pressed)
        {
            settingsDialog->exec();
            // key-release event may not be received after dialog is shown
            ctrl_pressed = false;
        }
        break;
    case Qt::Key_Left:
        if (ctrl_pressed)
            core->speedDown();
        else
            core->seek(-5, false);
        break;

    case Qt::Key_Right:
        if (ctrl_pressed)
            core->speedUp();
        else
            core->seek(5, false);
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

void WindowBase::keyReleaseEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Control)
        ctrl_pressed = false;
    e->accept();
}


void WindowBase::contextMenuEvent(QContextMenuEvent *e)
{
    menu->exec(QCursor::pos());
    e->accept();
}

// show cutterbar
void WindowBase::showCutterBar()
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

void WindowBase::saveVolume(int vol)
{
    Settings::volume = vol;
}


// add & select subtitle and set subtitle delay
void WindowBase::addSubtitle()
{
    QString videoFile = core->currentFile();
    QString dir = videoFile.startsWith('/') ? QFileInfo(videoFile).path() : QDir::homePath();
    QString subFile = QFileDialog::getOpenFileName(this, tr("Open subtitle file"), dir);
    if (!subFile.isEmpty())
        core->openSubtitle(subFile);
}

void WindowBase::selectSubtitle()
{
    int sid = selectionDialog->showDialog_Index(core->getSubtitleList(), tr("Select subtitle:"));
    if (sid != -1)
        core->setSid(sid);
}

void WindowBase::setSubDelay()
{
    bool ok = false;
    double delay = QInputDialog::getDouble(this, "Input", tr("Subtitle delay (sec):"), core->getAudioDelay(), -100, 100, 1, &ok);
    if (ok)
        core->setSubDelay(delay);
}

// add audio track, select audio track and set audio delay
void WindowBase::addAudioTrack()
{
    QString videoFile = core->currentFile();
    QString dir = videoFile.startsWith('/') ? QFileInfo(videoFile).path() : QDir::homePath();
    QString audioFile = QFileDialog::getOpenFileName(this, tr("Open audio track file"), dir);
    if (!audioFile.isEmpty())
        core->openAudioTrack(audioFile);
}

void WindowBase::selectAudioTrack()
{
    int aid = selectionDialog->showDialog_Index(core->getAudioTracksList(), tr("Select audio track:"));
    if (aid != -1)
        core->setAid(aid);
}

void WindowBase::setAudioDelay()
{
    bool ok = false;
    double delay = QInputDialog::getDouble(this, "Input", tr("Audio delay (sec):"), core->getAudioDelay(), -100, 100, 1, &ok);
    if (ok)
        core->setAudioDelay(delay);
}


//open extension page
void WindowBase::openExtPage()
{
    static QUrl url("https://github.com/coslyk/moonplayer/wiki/BrowserExtension");
    QDesktopServices::openUrl(url);
}
