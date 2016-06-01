#include "skin.h"
#include "settings_player.h"
#include <QWidget>
#include <QMouseEvent>

//window's borders
Border::Border(QWidget *topwin, BorderType t) :
    QWidget(0)
{
    type = t;
    topwindow = topwin;
    if (type == BOTTOM)
        setFixedHeight(4 * Settings::uiScale);
    else
        setFixedWidth(4 * Settings::uiScale);
}

void Border::enterEvent(QEvent *)
{
    if (type == BOTTOM)
        setCursor(Qt::SizeVerCursor);
    else
        setCursor(Qt::SizeHorCursor);
}

void Border::mousePressEvent(QMouseEvent *e)
{
    oldPos = e->globalPos();
}

void Border::mouseMoveEvent(QMouseEvent *e)
{
    int dx = e->globalX() - oldPos.x();
    int dy = e->globalY() - oldPos.y();
    QRect g = topwindow->geometry();

    if (type == BOTTOM)
        g.setBottom(g.bottom() + dy);
    else if (type == LEFT)
        g.setLeft(g.left() + dx);
    else if (type == RIGHT)
        g.setRight(g.right() + dx);
    topwindow->setGeometry(g);
    oldPos = e->globalPos();
}

void Border::leaveEvent(QEvent *)
{
    setCursor(Qt::ArrowCursor);
}
