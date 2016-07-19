#ifndef MPLAYER_H
#define MPLAYER_H

#include <QWidget>
#include <QHash>
class QTimer;
class QMenu;
class QProcess;
class QResizeEvent;
class QLabel;
#ifdef Q_OS_LINUX
class DanmakuLoader;
#endif
#ifdef Q_OS_MAC
#include <QMediaPlayer>
#include <QVideoWidget>
#endif

#ifdef Q_OS_MAC
class PlayerCore : public QVideoWidget
#else
class PlayerCore : public QWidget
#endif
{
    Q_OBJECT

signals:
    void cutVideo(void);
    void played(void);
    void paused(void);
    void stopped(void);
    void fullScreen(void);
    void timeChanged(int pos);
    void lengthChanged(int len);
    void sizeChanged(const QSize &size);

public:
    typedef enum {STOPPING, VIDEO_PLAYING, VIDEO_PAUSING, TV_PLAYING} State;
    typedef enum {CHANNEL_NORMAL, CHANNEL_LEFT,CHANNEL_RIGHT} Channel;

    explicit PlayerCore(QWidget *parent = 0);
    ~PlayerCore();
    State state;
    Channel channel;

public slots:
    void stop(void);
    void changeState(void);
    void jumpTo(int pos);
    void setProgress(int pos);
    void setVolume(int percentage);
    void openFile(const QString &file, const QString &danmaku = QString());
    void screenShot(void);
    void speedUp(void);
    void speedDown(void);
    void speedSetToDefault(void);

protected:
    void mouseDoubleClickEvent(QMouseEvent *);

#ifndef Q_OS_MAC
public:
    inline QWidget *getLayer() {return layer;}
    inline int getTime() {return progress;}
    inline int getLength() {return length;}
    inline QString currentFile() {return wait_to_play;}

protected:
    void resizeEvent(QResizeEvent *);

private slots:
    void onFinished(int);
    void readOutput(void);
    void updateTime(void);
    void showMenu(const QPoint&);
    void playWaiting(void);
    void setRatio_16_9(void);
    void setRatio_16_10(void);
    void setRatio_4_3(void);
    void setRatio_0(void);
    void setChannelToLeft(void);
    void setChannelToRight(void);
    void setChannelToNormal(void);
    void switchDanmaku(void);
    void loadAss(const QString &assFile);

private:
    QProcess* process; //player_core process
    QWidget* layer;  //Window for video output
    QMenu* menu;
    QLabel* msgLabel;  //Show catching message
    DanmakuLoader* danmakuLoader;

    QAction* leftChannelAction;
    QAction* rightChannelAction;
    QAction* normalChannelAction;
    QAction* screenShotAction;
    QAction* switchDanmakuAction;

    QTimer* timer;
    int volume;
    int progress;
    int length;
    int time_offset;
    int w, h;
    double speed;
    bool is_waiting;
    bool stop_called;
    QString wait_to_play;
    QString playing_file;
    QString danmaku;
    QHash<QString, int> unfinished_time;

    void cb_start(const QString &msg);
    void cb_ratioChanged(const QString &msg);
    void cb_updateTime(const QString &msg);
    void resizeLayer(void);
    void writeToMplayer(const QByteArray &msg);

#else
public:
    inline int getTime() {return mediaPlayer->position() / 1000;}
    inline int getLength() {return mediaPlayer->duration() / 1000;}
    inline QString currentFile() {return currentVideo;}
    inline QWidget *getLayer() {return NULL;}

private:
    QMediaPlayer *mediaPlayer;
    QString currentVideo;
    QSize currentRelution;
    int currentPos;
    bool switchingVideo;

private slots:
    void onDurationChanged(qint64 duration);
    void onMetaDataChanged(const QString &key, const QVariant &value);
    void onPositionChanged(qint64 position);
    void onStateChanged(QMediaPlayer::State state);
#endif
};

extern PlayerCore *player_core;

#endif // MPLAYER_H
