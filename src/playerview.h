#ifndef PLAYERVIEW_H
#define PLAYERVIEW_H

#include <QPoint>
#include <QWidget>

namespace Ui {
class PlayerView;
}
class Border;
class Playlist;
class PlayerCore;
class QSlider;
class QTimer;

class PlayerView : public QWidget
{
    Q_OBJECT

public:
    explicit PlayerView(QWidget *parent = 0);
    ~PlayerView();

protected:
    void closeEvent(QCloseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);
    void resizeEvent(QResizeEvent *e);

private slots:
    void onLengthChanged(int len);
    void onTimeChanged(int time);
    void onSizeChanged(const QSize &sz);
    void onStopButton(void);
    void onStopped(void);
    void showHidePlaylist(void);
    void showVolumeSlider(void);

private:
    Ui::PlayerView *ui;
    Border *leftBorder;
    Border *rightBorder;
    Border *bottomBorder;
    Playlist *playlist;
    PlayerCore *core;
    QSlider *volumeSlider;
    QTimer *hideTimer;
    QPoint dPos;
    bool quit_requested;
    bool no_play_next;
};

#endif // PLAYERVIEW_H
