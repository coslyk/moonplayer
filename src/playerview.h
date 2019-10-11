#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <QWidget>

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

class PlayerView : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerView(QWidget *parent = 0);
    ~PlayerView();

protected:
    void contextMenuEvent(QContextMenuEvent *e);
    void closeEvent(QCloseEvent *e);
    void dragEnterEvent(QDragEnterEvent *e);
    void dropEvent(QDropEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void keyReleaseEvent(QKeyEvent *e);

protected slots:
    void onStopButton(void);

private slots:
    void onSizeChanged(const QSize &sz);
    void onStopped(void);
    void addAudioTrack(void);
    void addSubtitle(void);
    void selectAudioTrack(void);
    void selectSubtitle(void);
    void setAudioDelay(void);
    void setSubDelay(void);
    void saveVolume(int vol);
    void showCutterBar(void);
    void openExtPage(void);

protected:
    AboutDialog *aboutDialog;
    CutterBar *cutterBar;
    Playlist *playlist;
    PlayerCore *core;
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
