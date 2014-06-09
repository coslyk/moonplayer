#ifndef SKIN_H
#define SKIN_H

#include <QWidget>
class QString;
class QPushButton;
class QPixmap;

namespace Skin {

void setWidgetBG(QWidget* widget, const QString& img);
void setSlider(QWidget* slider, const QString& bg, const QString& handle);
void setButton(QPushButton* button, const QPixmap& img);
void setButton(QPushButton *button, const QString &normal, const QString &hover,
               const QString &pressed = QString(), const QString &text = QString());
void setListView(QWidget *listview, const QString &bg, const QString &selected, const QString &inactive,
                 const QString &scrollbar, const QString &scrollbg, const QString &hscrollbar, const QString &hscrollbg);
}

class Border : public QWidget
{
public:
    enum BorderType{LEFT, RIGHT, BOTTOM};
    Border(QWidget* topwin, BorderType type);
    void setPicture(const QPixmap&);
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
