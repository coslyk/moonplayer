#ifndef SKIN_H
#define SKIN_H

#include <QWidget>
class QString;
class QPushButton;
class QPixmap;

class Border : public QWidget
{
public:
    enum BorderType{LEFT, RIGHT, BOTTOM};
    Border(QWidget* topwin, BorderType type);
protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
private:
    BorderType type;
    bool mouseDown;
    QWidget* topwindow;
    QPoint oldPos;
};

#endif // SKIN_H
