#ifndef MODERNWINDOW_H
#define MODERNWINDOW_H

#include "playerview.h"
#include <QPoint>

namespace Ui {
class PlayerView;
}
class Border;
class QTimer;

class ModernWindow : public PlayerView
{
    Q_OBJECT
public:
    explicit ModernWindow(QWidget *parent = nullptr);
    ~ModernWindow();

protected:
    void changeEvent(QEvent *e);
    void keyPressEvent(QKeyEvent *e);
    void resizeEvent(QResizeEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);
    void mousePressEvent(QMouseEvent *e);
    void mouseMoveEvent(QMouseEvent *e);
    void mouseReleaseEvent(QMouseEvent *e);

private slots:
    void hideElements(void);
    void onMaxButton(void);
    void onLengthChanged(int len);
    void onTimeChanged(int time);
    void onTimeSliderPressed(void);
    void onTimeSliderValueChanged(int time);
    void onTimeSliderReleased(void);
    void setFullScreen(void);
    void showPlaylist(void);
    void showVolumeSlider(void);

private:
    QPoint dPos;
    Ui::PlayerView *ui;
    QTimer *hideTimer;
    Border *leftBorder;
    Border *rightBorder;
    Border *bottomBorder;
    Border *bottomLeftBorder;
    Border *bottomRightBorder;
};

#endif // MODERNWINDOW_H
