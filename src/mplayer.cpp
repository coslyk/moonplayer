#include "mplayer.h"
#include "settings_video.h"
#include "settings_network.h"
#include <QProcess>
#include <QColor>
#include <QSize>
#include <QPalette>
#include <QTimer>
#include <QMenu>
#include <QCursor>
#include <QMouseEvent>
#include <QLabel>
#include <QDir>
#include <QKeySequence>
#include <QFileDialog>
#include <iostream>
using namespace std;

MPlayer *mplayer = NULL;

MPlayer::MPlayer(QWidget *parent) :
    QWidget(parent)
{
    std::cout << "Initialize mplayer backend..." << std::endl;
    w = h = 1;

    //Create timer
    timer = new QTimer(this);

    //Create layer
    layer = new QWidget(this);
    layer->setFocusPolicy(Qt::StrongFocus);
    setFocusPolicy(Qt::StrongFocus);

    //Create catch message
    msgLabel = new QLabel(layer);
    msgLabel->move(0, 0);
    msgLabel->resize(400, 15);
    msgLabel->hide();

    //Set state
    state = STOPPING;
    is_waiting = false;
    stop_called = false;
    volume = 100;

    //Set color
    setPalette(QPalette(QColor(0, 0, 0)));
    setAutoFillBackground(true);

    //Create mplayer process
    process = new QProcess(this);
    process->setWorkingDirectory(QDir::homePath());
    process->setProcessChannelMode(QProcess::MergedChannels);
    connect(process, SIGNAL(finished(int)), this, SLOT(onFinished(int)));
    connect(process, SIGNAL(readyReadStandardOutput()), this, SLOT(readOutput()));
    connect(timer, SIGNAL(timeout()), this, SLOT(updateTime()));

    //set menu
    QMenu *ratio_menu = new QMenu(tr("Ratio"));
    ratio_menu->addAction("4:3", this, SLOT(setRatio_4_3()));
    ratio_menu->addAction("16:9", this, SLOT(setRatio_16_9()));
    ratio_menu->addAction("16:10", this, SLOT(setRatio_16_10()));
    ratio_menu->addAction(tr("Default"), this, SLOT(setRatio_0()));

    QMenu *speed_menu = new QMenu(tr("Speed"));
    speed_menu->addAction(tr("Speed up"), this, SLOT(speedUp()), QKeySequence("Ctrl+Right"));
    speed_menu->addAction(tr("Speed down"), this, SLOT(speedDown()), QKeySequence("Ctrl+Left"));
    speed_menu->addAction(tr("Default"), this, SLOT(speedSetToDefault()), QKeySequence("R"));

    QMenu *channel_menu = new QMenu(tr("Channel"));
    leftChannelAction   = channel_menu->addAction(tr("Left"), this, SLOT(setChannelToLeft()));
    rightChannelAction  = channel_menu->addAction(tr("Right"), this, SLOT(setChannelToRight()));
    normalChannelAction = channel_menu->addAction(tr("Normal"), this, SLOT(setChannelToNormal()));
    leftChannelAction->setCheckable(true);
    rightChannelAction->setCheckable(true);
    normalChannelAction->setCheckable(true);
    normalChannelAction->setChecked(true);
    channel = CHANNEL_NORMAL;

    menu = new QMenu(this);
    screenShotAction = menu->addAction(tr("Screenshot"), this, SLOT(screenShot()), QKeySequence("S"));
    menu->addSeparator();
    menu->addMenu(ratio_menu);
    menu->addMenu(speed_menu);
    menu->addMenu(channel_menu);
    //menu->addAction(tr("Load subtitles"), this, SLOT(loadSub())); //Unfinished function
    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint&)),
            this, SLOT(showMenu(const QPoint&)));

#ifdef Q_OS_LINUX
    // read unfinished_time
    QString filename = QDir::homePath() +"/.moonplayer/unfinished.txt";
    QFile file(filename);
    if (!file.open(QFile::ReadOnly))
        return;
    QByteArray data = file.readAll();
    file.close();
    if (data.isEmpty())
        return;
    QStringList list = QString::fromUtf8(data).split('\n');
    for (int i = 0; i < list.size(); i += 2)
        unfinished_time[list[i]] = list[i + 1].toInt();
#endif
    mplayer = this;
}


MPlayer::~MPlayer()
{
#ifdef Q_OS_LINUX
    if (!unfinished_time.isEmpty() && Settings::rememberUnfinished)
    {
        QByteArray data;
        QHash<QString, int>::const_iterator i = unfinished_time.constBegin();
        while (i != unfinished_time.constEnd())
        {
            QString name = i.key();
            if (!name.startsWith("http://"))
                data += name.toUtf8() + '\n' + QByteArray::number(i.value()) + '\n';
            i++;
        }
        data.chop(1); // Remove last '\n'
        if (data.isEmpty())
            return;
        QString filename = QDir::homePath() + "/.moonplayer/unfinished.txt";
        QFile file(filename);
        if (!file.open(QFile::WriteOnly))
            return;
        file.write(data);
        file.close();
    }
    else
    {
        QDir dir = QDir::home();
        dir.cd(".moonplayer");
        if (dir.exists("unfinished.txt"))
            dir.remove("unfinished.txt");
    }
#endif
}

//resize layer when window size changes
void MPlayer::resizeEvent(QResizeEvent *)
{
    if (state != STOPPING)
        resizeLayer();
}

void MPlayer::resizeLayer()
{
    int pos_x, pos_y, size_width, size_height;
    int win_width = width();
    int win_height = height();

    size_width = win_width;
    size_height = size_width * h / w;
    if (size_height > win_height)
    {
        size_height = win_height;
        size_width = size_height * w / h;
    }

    pos_x = (win_width - size_width) / 2;
    pos_y = (win_height - size_height) / 2;

    layer->move(pos_x, pos_y);
    layer->resize(size_width, size_height);
}

/*Set mplayer to fullscreen*/
void MPlayer::mouseDoubleClickEvent(QMouseEvent* e)
{
    if (e->buttons() == Qt::LeftButton)
        emit fullScreen();
}

/* Open a new video file. MPlayer::openFile sends a message to get length
 * and the result will be read in MPlayer::cb_start.
 */
void MPlayer::openFile(const QString& filename)
{
    static QString proxyUrl = "http_proxy://%1:%2/%3";

    //show debug message label
    msgLabel->show();
    //start MPlayer if stopping
    if (state != STOPPING)
    {
        wait_to_play = filename;
        is_waiting = true;
        stop();
        return;
    }

    playing_file = filename;
    //Set time to 0 and cache state
    emit timeChanged(0);
    is_mplayer2 = false;

    QStringList args;
    args << "-quiet";
    args << "-slave";
    args << "-vo" << Settings::vout;
#ifdef Q_OS_LINUX
    if (Settings::vout == "vdpau")
    {
        args << "-vc";
        if (Settings::ffodivxvdpau)
            args << "ffmpeg12vdpau,ffwmv3vdpau,ffvc1vdpau,ffh264vdpau,ffodivxvdpau,";
        else
            args << "ffmpeg12vdpau,ffwmv3vdpau,ffvc1vdpau,ffh264vdpau,";
    }
#endif
    args << "-volume" << QString::number(volume);
    args << "-wid" << QString::number((unsigned long) layer->winId());
    if (filename.startsWith("http://"))
    {
        args << "-user-agent" << "moonplayer";
        args << "-cache" << QString::number(Settings::cacheSize);
        args << "-cache-min" << QString::number(Settings::cacheMin);
    }

    //set channels
    if (channel == CHANNEL_LEFT)
        args << "-af" << "channels=2:2:0:1:0:0";
    else if (channel == CHANNEL_RIGHT)
        args << "-af" << "channels=2:2:1:0:1:1";

    //set video arguments
    if (Settings::enableScreenshot)
    {
        args << "-vf-add" << "screenshot";
        screenShotAction->setEnabled(true);
    }
    else
        screenShotAction->setEnabled(false);

    args << (Settings::doubleBuffer ? "-double" : "-nodouble");
    if (Settings::framedrop)
        args << "-framedrop";

    // Start at the previous stopping time
    time_offset = -1;
    if (unfinished_time.contains(filename) && Settings::rememberUnfinished)
    {
        args << "-ss" << QString::number(unfinished_time[filename]);
        time_offset = 0; // only videos with no time offset can be remembered
    }

    //set proxy and open file
    if (!Settings::proxy.isEmpty() && filename.startsWith("http://")) //Set proxy
        args << proxyUrl.arg(Settings::proxy, QString::number(Settings::port),
                             filename);
    else
        args << filename;
    //set state
    state = TV_PLAYING; //If playing video, state will reset later in MPlayer::cb_start()
    length = 0;
    progress = 0;
    speed = 1.0;

    //start
    process->start("mplayer", args);
}

void MPlayer::cb_start(QString& msg)
{
    float l = msg.section('=', 1, 1).simplified().toFloat();
    if (l != 0.0f) //playing video, not TV
    {
        state = VIDEO_PLAYING;
        timer->start(1000);
    }
    length = (int) l;
    progress = 0;
    emit lengthChanged(length);
    emit played();
    //hide debug message label
    msgLabel->hide();
}


/* Change state between pausing and playing. */
void MPlayer::changeState()
{
    if (state == STOPPING || state == TV_PLAYING)
        return;
    process->write("pause\n");
    switch (state)
    {
    case VIDEO_PLAYING:
        timer->stop();
        state = VIDEO_PAUSING;
        emit paused();
        break;
    case VIDEO_PAUSING:
        timer->start(1000);
        state = VIDEO_PLAYING;
        emit played();
        break;
    default:break;
    }
}


/* MPlayer::stop sends a message to quit mplayer. When the mplayer
 * receive the message or the video is at the end, it will automatically
 * exit and emit Finished() signal. Then something will be done
 * in MPlayer::onFinished.
 */
void MPlayer::stop()
{
    stop_called = true;
    if (state != STOPPING)
        process->write("stop\n");
    process->waitForFinished(2000);
    if (process->state() == QProcess::Running)
        process->kill();
}

void MPlayer::onFinished(int)
{
    state = STOPPING;
    timer->stop();

    // Remember the unfinished time
    if (progress < length - 1 && time_offset == 0)
        unfinished_time[playing_file] = progress;
    else
        unfinished_time.remove(playing_file);

    // Check whether mplayer quits abnormally
    if (progress < length - 1 && !stop_called)
    {
        is_waiting = true;
        wait_to_play = playing_file;
    }
    stop_called = false;

    if (is_waiting)
        //play after event loop
        QTimer::singleShot(0, this, SLOT(playWaiting()));
    else
        emit stopped();
}

void MPlayer::playWaiting()
{
    is_waiting = false;
    openFile(wait_to_play);
}

void MPlayer::setVolume(int vol)
{
    char msg[16];
    volume = vol * 10;
    if (state == STOPPING)
        return;

    qsnprintf(msg, 16, "volume %d 1\n", volume);
    writeToMplayer(msg);
}

void MPlayer::readOutput()
{
    while (process->canReadLine())
    {
        QString message = process->readLine();
        QString format = "<span style=\"color:#ff0000;\">%1</span>";

        if (message.startsWith("Starting playback")) //Read file done
            writeToMplayer("get_time_length\n");

        else if (message.startsWith("VO:"))
            cb_ratioChanged(message);

        else if (message.startsWith("ANS_LENGTH="))
            cb_start(message);

        else if (message.startsWith("ANS_TIME_POSITION="))
            cb_updateTime(message);

        // Windows version is based on Mplayer(not Mplayer2) since v0.22
#ifdef Q_OS_LINUX
        else if (message.toLower().startsWith("mplayer2"))
            is_mplayer2 = true;
#endif

        else if (msgLabel->isVisible())
        {
            msgLabel->setText(format.arg(message));
        }
    }
}

void MPlayer::writeToMplayer(const QByteArray &msg)
{
    if (!is_mplayer2 && state == VIDEO_PAUSING)
        process->write("pausing_keep_force " + msg);
    else
        process->write(msg);
}

/*Update time every 1s. MPlayer::updateTime() will send a
 * message to get position and the result will be read in
 * MPlayer::cb_changeState.
 */
void MPlayer::updateTime()
{
    writeToMplayer("get_time_pos\n");
}


void MPlayer::cb_updateTime(QString& msg)
{
    int pos = msg.section('=', 1, 1).section('.', 0, 0).toInt();
    //fix mplayer's error
    if (time_offset == -1) //first time to be called
        time_offset = (pos > 1) ? pos - 1 : 0;
    pos -= time_offset;
    if (Settings::fixLastFrame)
    {
        if (pos == progress && pos >= length)
        {
            stop();
            return;
        }
    }
    progress = pos;
    if (progress % UPDATE_FREQUENCY == 0)
        emit timeChanged(progress);
}

/*set progress*/
void MPlayer::setProgress(int pos)
{
    //Ignore if stopping or playing tv
    if (state != VIDEO_PLAYING && state != VIDEO_PAUSING)
        return;

    //Ignore if the time is equal to the progress
    if (pos == progress)
        return;

    //Set progress
    progress = pos;
    char msg[16];
    qsnprintf(msg, 16, "seek %d 2\n", pos + time_offset);
    writeToMplayer(msg);
}

//set video's ratio
void MPlayer::setRatio_16_9()
{
    static QByteArray msg = "switch_ratio " + QByteArray::number(16.0 / 9.0) + '\n';
    if (state == STOPPING)
        return;
    writeToMplayer(msg);
}

void MPlayer::setRatio_16_10()
{
    if (state == STOPPING)
        return;
    writeToMplayer("switch_ratio 1.6\n");
}

void MPlayer::setRatio_4_3()
{
    static QByteArray msg = "switch_ratio " + QByteArray::number(4.0 / 3.0) + '\n';
    if (state == STOPPING)
        return;
    writeToMplayer(msg);
}

void MPlayer::setRatio_0()
{
    if (state == STOPPING)
        return;
    writeToMplayer("switch_ratio 0\n");
}

void MPlayer::cb_ratioChanged(QString& msg)
{
    QStringList size = msg.section(' ', 4, 4).split('x');
    w = size.takeFirst().toInt();
    h = size.takeFirst().toInt();
    QSize sz(w, h);
    emit sizeChanged(sz);
}

//Show right-button menu
void MPlayer::showMenu(const QPoint&)
{
    menu->exec(QCursor::pos());
}

//Set channel
void MPlayer::setChannelToLeft()
{
    leftChannelAction->setChecked(true);
    rightChannelAction->setChecked(false);
    normalChannelAction->setChecked(false);
    channel = CHANNEL_LEFT;
    if (state != STOPPING)
    {
        is_waiting = true;
        stop();
    }
}

void MPlayer::setChannelToRight()
{
    leftChannelAction->setChecked(false);
    rightChannelAction->setChecked(true);
    normalChannelAction->setChecked(false);
    channel = CHANNEL_RIGHT;
    if (state != STOPPING)
    {
        is_waiting = true;
        stop();
    }
}

void MPlayer::setChannelToNormal()
{
    leftChannelAction->setChecked(false);
    rightChannelAction->setChecked(false);
    normalChannelAction->setChecked(true);
    channel = CHANNEL_NORMAL;
    if (state != STOPPING)
    {
        is_waiting = true;
        stop();
    }
}

// Screenshot
void MPlayer::screenShot()
{
    if (state != STOPPING && screenShotAction->isEnabled())
    {
        writeToMplayer("screenshot 0\n");
        writeToMplayer("osd_show_text \"Screenshot has been saved.\" 3000 1\n");
    }
}

// Load subtitles
void MPlayer::loadSub()
{
    if (state == STOPPING)
        return;
    if (state == TV_PLAYING || state == VIDEO_PLAYING) //pause first
        changeState();
    QString file = QFileDialog::getOpenFileName(this, "Select subtitles file");
    changeState(); //continue playing
    if (!file.isEmpty())
        writeToMplayer("sub_load " + file.toLocal8Bit() + '\n');
}

// Set speed
void MPlayer::speedUp()
{
    if (speed < 2.0 && state != STOPPING)
    {
        speed += 0.1;
        writeToMplayer("speed_set " + QByteArray::number(speed) + '\n');
    }
}

void MPlayer::speedDown()
{
    if (speed > 0.5 && state != STOPPING)
    {
        speed -= 0.1;
        writeToMplayer("speed_set " + QByteArray::number(speed) + '\n');
    }
}

void MPlayer::speedSetToDefault()
{
    if (state != STOPPING)
    {
        speed = 1;
        writeToMplayer("speed_set 1\n");
    }
}
