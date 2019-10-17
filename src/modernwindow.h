#ifndef MODERNWINDOW_H
#define MODERNWINDOW_H

#include "windowbase.h"
#include <QPoint>

namespace Ui {
class ModernWindow;
}
class Border;
class QTimer;

class ModernWindow : public WindowBase
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
    bool eventFilter(QObject *watched, QEvent *event);

protected slots:
    void showPlaylist(void);

private slots:
    void hideElements(void);
    void onMaxButton(void);
    void onLengthChanged(int len);
    void onSizeChanged(const QSize &sz);
    void onTimeChanged(int time);
    void onTimeSliderPressed(void);
    void onTimeSliderValueChanged(int time);
    void onTimeSliderReleased(void);
    void setFullScreen(void);
    void showVolumeSlider(void);

private:
    QPoint dPos;
    Ui::ModernWindow *ui;
    QTimer *hideTimer;
    Border *leftBorder;
    Border *rightBorder;
    Border *bottomBorder;
    Border *bottomLeftBorder;
    Border *bottomRightBorder;
};

#endif // MODERNWINDOW_H
