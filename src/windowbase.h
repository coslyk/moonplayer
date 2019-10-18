#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <QMainWindow>

class AboutDialog;
class CutterBar;
class Playlist;
class PlayerCore;
class QMenu;
class QSlider;
class ResLibrary;
class SelectionDialog;
class SettingsDialog;
class UpgraderDialog;

namespace Ui {
class Equalizer;
}

class WindowBase : public QMainWindow
{
    Q_OBJECT

public:
    explicit WindowBase(QWidget *parent = 0);
    ~WindowBase();

protected:
    void contextMenuEvent(QContextMenuEvent *e);
    void closeEvent(QCloseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

protected slots:
    void onStopButton(void);
    void openExtPage(void);
    void showCutterBar(void);
    virtual void showPlaylist(void) = 0;

private slots:
    void onStopped(void);
    void addAudioTrack(void);
    void addSubtitle(void);
    void selectAudioTrack(void);
    void selectSubtitle(void);
    void setAudioDelay(void);
    void setSubDelay(void);
    void saveVolume(int vol);

protected:
    AboutDialog *aboutDialog;
    CutterBar *cutterBar;
    Playlist *playlist;
    PlayerCore *core;
    QDialog *equalizer;
    Ui::Equalizer *equalizer_ui;
    QMenu *menu;
    QSlider *volumeSlider;
    ResLibrary *reslibrary;
    SelectionDialog *selectionDialog;
    SettingsDialog *settingsDialog;
    UpgraderDialog *upgraderDialog;
    bool quit_requested;
    bool no_play_next;
    bool ctrl_pressed;
};

#endif // PLAYERVIEW_H
