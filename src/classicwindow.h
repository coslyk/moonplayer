#ifndef CLASSICWINDOW_H
#define CLASSICWINDOW_H

#include "windowbase.h"

namespace Ui {
class ClassicWindow;
}
class PlayerCore;
class QSlider;

class ClassicWindow : public WindowBase
{
    Q_OBJECT
public:
    explicit ClassicWindow(QWidget *parent = nullptr);
    ~ClassicWindow();

protected:
    void changeEvent(QEvent *e);
    bool eventFilter(QObject *watched, QEvent *event);
    void keyPressEvent(QKeyEvent *e);
    void mouseDoubleClickEvent(QMouseEvent *e);

protected slots:
    void showPlaylist(void);

private slots:
    void onLengthChanged(int len);
    void onSizeChanged(const QSize &video_size);
    void onTimeChanged(int time);
    void onTimeSliderPressed(void);
    void onTimeSliderValueChanged(int time);
    void onTimeSliderReleased(void);
    void setFullScreen(void);
    void showVolumeSlider(void);

private:
    Ui::ClassicWindow *ui;
};

#endif // CLASSICWINDOW_H
