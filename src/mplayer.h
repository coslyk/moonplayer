#ifndef MPLAYER_H
#define MPLAYER_H

#include <QWidget>
#include <QHash>
class QTimer;
class QMenu;
class QProcess;
class QResizeEvent;
class QLabel;

class MPlayer : public QWidget
{
    Q_OBJECT

signals:
    void played(void);
    void paused(void);
    void stopped(void);
    void fullScreen(void);
    void timeChanged(int pos);
    void lengthChanged(int len);
    void sizeChanged(QSize &size);

public:
    typedef enum {STOPPING, VIDEO_PLAYING, VIDEO_PAUSING, TV_PLAYING} MPlayerState;
    typedef enum {CHANNEL_NORMAL, CHANNEL_LEFT,CHANNEL_RIGHT} Channel;

    explicit MPlayer(QWidget *parent = 0);
    ~MPlayer();
    MPlayerState state;
    Channel channel;
    static const int UPDATE_FREQUENCY = 5; //Update the progress every 5s.
    inline QWidget* getLayer(){return layer;}
    inline int getTime(){return progress;}
    inline QString currentFile(){return wait_to_play;}

public slots:
    void stop(void);
    void changeState(void);
    void setProgress(int pos);
    void setVolume(int percentage);
    void openFile(const QString&);
    void screenShot(void);
    void speedUp(void);
    void speedDown(void);
    void speedSetToDefault(void);

protected:
    void resizeEvent(QResizeEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);

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

private:
    QProcess* process; //mplayer process
    QWidget* layer;  //Window for video output
    QLabel* msgLabel;  //Show catching message

    QMenu* menu;
    QAction* leftChannelAction;
    QAction* rightChannelAction;
    QAction* normalChannelAction;
    QAction* screenShotAction;

    QTimer* timer;
    int volume;
    int progress;
    int length;
    int time_offset;
    int w, h;
    float speed;
    bool is_waiting;
    bool stop_called;
    bool is_mplayer2;
    QString wait_to_play;
    QString playing_file;
    QHash<QString, int> unfinished_time;

    void cb_start(QString& msg);
    void cb_ratioChanged(QString& msg);
    void cb_updateTime(QString& msg);
    void resizeLayer(void);
    void writeToMplayer(const QByteArray &msg);
};

extern MPlayer *mplayer;

#endif // MPLAYER_H
