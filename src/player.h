#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
class MPlayer;
class Playlist;
class QCloseEvent;
class WebVideo;
class QHBoxLayout;
class QMouseEvent;
class Border;
class QLabel;
class QMenu;
class Downloader;
class Transformer;

namespace Ui {
class Player;
}


class Player : public QWidget
{
    Q_OBJECT
    
public:
    explicit Player(QWidget *parent = 0);
    void setSkin(const QString& skin_name);
    ~Player();
    Playlist* playlist;

protected:
    void closeEvent(QCloseEvent* e);
    bool eventFilter(QObject *, QEvent *);

private:
    Ui::Player *ui;
    MPlayer* mplayer;
    WebVideo* webvideo;
    Downloader *downloader;
    Transformer *transformer;
    //borders
    Border* leftBorder;
    Border* rightBorder;
    Border* bottomBorder;
    Border* topLeftBorder;
    Border* topRightBorder;
    QLabel* timeShow;
    QMenu* menu;

    enum SKIN{
        SKIN_PLAY,
        SKIN_STOP,
        SKIN_PAUSE,
        SKIN_ADD,
        SKIN_NUM
    };
    QPixmap skin[SKIN_NUM];
    bool no_play_next;
    bool is_fullscreen;
    bool toolbar_visible;
    int toolbar_pos_y;

private slots:
    void setIconToPlay(void);
    void setIconToPause(void);
    void setMaxNormal(void);
    void showMenu(void);
    void onNeedPause(bool);
    void onLengthChanged(int);
    void onProgressChanged(int);
    void onPBarChanged(int);
    void onPBarPressed(void);
    void onPBarReleased(void);
    void onSizeChanged(QSize &size);
    void onStopped(void);
    void onStopButton(void);
    void onSetButton(void);
    void openHomepage(void);

    void hidePlaylist(void);
    void setFullScreen(void);
};

#endif // PLAYER_H
