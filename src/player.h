#ifndef PLAYER_H
#define PLAYER_H

#include <QWidget>
#include <QIcon>
class MPlayer;
class Playlist;
class QCloseEvent;
class WebVideo;
class ResLibrary;
class QHBoxLayout;
class QMouseEvent;
class Border;
class QLabel;
class QMenu;
class Downloader;
class Transformer;
class SettingsDialog;
class CutterBar;

namespace Ui {
class Player;
}


class Player : public QWidget
{
    Q_OBJECT
    
public:
    explicit Player(QWidget *parent = 0);
    ~Player();
    Playlist* playlist;

protected:
    void closeEvent(QCloseEvent* e);
    bool eventFilter(QObject *, QEvent *);

private:
    Ui::Player *ui;
    MPlayer* mplayer;
    WebVideo* webvideo;
    ResLibrary *reslibrary;
    Downloader *downloader;
    Transformer *transformer;
    SettingsDialog *settingsDialog;
    CutterBar *cutterbar;
    //borders
    Border* leftBorder;
    Border* rightBorder;
    Border* bottomBorder;
    Border* topLeftBorder;
    Border* topRightBorder;
    QLabel* timeShow;
    QMenu* menu;

    bool no_play_next;
    bool is_fullscreen;
    bool toolbar_visible;
    bool mouse_in_toolbar;
    bool progressbar_pressed;
    int toolbar_pos_y;

    void setSkin(const QString& skin_name);

private slots:
    void setIconToPlay(void);
    void setIconToPause(void);
    void setMaxNormal(void);
    void showMenu(void);
    void showCutterbar(void);
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
    void onSaveVolume(int volume);
    void openExtPage(void);
    void openHomepage(void);
    void setFullScreen(void);
};

#endif // PLAYER_H
