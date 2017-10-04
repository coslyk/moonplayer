#ifndef SKIN_H
#define SKIN_H

#include <QWidget>
class QString;
class QPushButton;
class QPixmap;

// scale stylesheet
QString scaleStyleSheet(const QString qss);

class Border : public QWidget
{
public:
    enum BorderType{LEFT, RIGHT, BOTTOM, BOTTOMLEFT, BOTTOMRIGHT};
    Border(QWidget* topwin, BorderType type);
protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
private:
    BorderType type;
    QWidget* topwindow;
    QPoint oldPos;
};

#endif // SKIN_H
