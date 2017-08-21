#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <QPoint>
#include <QWidget>

namespace Ui {
class PlayerView;
}
class Border;
class CutterBar;
class Playlist;
class PlayerCore;
class QMenu;
class QSlider;
class QTimer;
class ResLibrary;
class SelectionDialog;
class SettingsDialog;

class PlayerView : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerView(QWidget *parent = 0);
    ~PlayerView();

protected:
#ifdef Q_OS_MAC
    void changeEvent(QEvent *e);
#endif
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
    void addSubtitle(void);
    void selectAudioTrack(void);
    void selectSubtitle(void);
    void saveVolume(int vol);
    void setFullScreen(void);
    void showCutterBar(void);
    void showPlaylist(void);
    void showVolumeSlider(void);
    void hideElements(void);
    void openHomepage(void);
    void openContributePage(void);
    void openExtPage(void);

private:
    Ui::PlayerView *ui;
    Border *leftBorder;
    Border *rightBorder;
    Border *bottomBorder;
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
    bool quit_requested;
    bool no_play_next;
    bool ctrl_pressed;
};

#endif // PLAYERVIEW_H
