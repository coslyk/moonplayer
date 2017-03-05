#ifndef CLASSICPLAYER_H
#define CLASSICPLAYER_H

#include <QMainWindow>
class CutterBar;
class Downloader;
class PlayerCore;
class Playlist;
class SettingsDialog;
class WebVideo;

namespace Ui {
class ClassicPlayer;
}

class ClassicPlayer : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClassicPlayer(QWidget *parent = 0);
    ~ClassicPlayer();

protected:
    void closeEvent(QCloseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    bool eventFilter(QObject *, QEvent *);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

private:
    Ui::ClassicPlayer *ui;
    CutterBar *cutterbar;
    Downloader *downloader;
    PlayerCore *player_core;
    Playlist *playlist;
    SettingsDialog *settingsDialog;
    WebVideo *webvideo;
    bool no_play_next;
    bool quit_requested;
    bool ctrl_pressed;
    int toolbar_pos_y;

private slots:
    void onLengthChanged(int length);
    void onProgressChanged(int);
    void onPBarChanged(int);
    void onPBarPressed(void);
    void onPBarReleased(void);
    void onSizeChanged(const QSize &sz);
    void onStopButton(void);
    void onStopped(void);
    void saveVolume(int volume);
    void setFullScreen(void);
    void showCutterbar(void);
    void openHomepage(void);
    void openExtPage(void);
    void openPluginsPage(void);
    void openContributePage(void);
};

#endif // CLASSICPLAYER_H
