#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <QPoint>
#include <QWidget>

namespace Ui {
class PlayerView;
}
class AboutDialog;
class Border;
class Browser;
class CutterBar;
class Playlist;
class PlayerCore;
class QMenu;
class QSlider;
class QTimer;
class ResLibrary;
class SelectionDialog;
class SettingsDialog;
class UpgraderDialog;

class PlayerView : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerView(QWidget *parent = 0);
    ~PlayerView();

protected:
    void changeEvent(QEvent *e);
    void contextMenuEvent(QContextMenuEvent *e);
    void closeEvent(QCloseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);

private slots:
    void onLengthChanged(int len);
    void onTimeChanged(int time);
    void onTimeSliderPressed(void);
    void onTimeSliderValueChanged(int time);
    void onTimeSliderReleased(void);
    void onSizeChanged(const QSize &sz);
    void onMaxButton(void);
    void onStopButton(void);
    void onStopped(void);
    void addAudioTrack(void);
    void addSubtitle(void);
    void selectAudioTrack(void);
    void selectSubtitle(void);
    void setAudioDelay(void);
    void setSubDelay(void);
    void saveVolume(int vol);
    void setFullScreen(void);
    void showCutterBar(void);
    void showPlaylist(void);
    void showVolumeSlider(void);
    void hideElements(void);
    void openExtPage(void);

private:
    Ui::PlayerView *ui;
    AboutDialog *aboutDialog;
    Border *leftBorder;
    Border *rightBorder;
    Border *bottomBorder;
    Border *bottomLeftBorder;
    Border *bottomRightBorder;
    Browser *browser;
    CutterBar *cutterBar;
    Playlist *playlist;
    PlayerCore *core;
    QMenu *menu;
    QSlider *volumeSlider;
    QTimer *hideTimer;
    QPoint dPos;
    ResLibrary *reslibrary;
    SelectionDialog *selectionDialog;
    SettingsDialog *settingsDialog;
    UpgraderDialog *upgraderDialog;
    bool quit_requested;
    bool no_play_next;
    bool ctrl_pressed;
};

#endif // PLAYERVIEW_H
