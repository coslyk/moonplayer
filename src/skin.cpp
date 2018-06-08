#include "skin.h"
#include <QWidget>
#include <QMouseEvent>
#include <QRegularExpression>

//window's borders
Border::Border(QWidget *topwin, BorderType t) :
    QWidget(0)
{
    type = t;
    topwindow = topwin;
    switch (type) {
    case BOTTOM:
        setFixedHeight(4);
        break;
    case LEFT:
    case RIGHT:
        setFixedWidth(4);
        break;
    case BOTTOMLEFT:
    case BOTTOMRIGHT:
        setFixedSize(4, 4);
        break;
    default:
        break;
    }
}

void Border::enterEvent(QEvent *)
{
    switch (type) {
    case BOTTOM:
        setCursor(Qt::SizeVerCursor);
        break;
    case LEFT:
    case RIGHT:
        setCursor(Qt::SizeHorCursor);
        break;
    case BOTTOMLEFT:
        setCursor(Qt::SizeBDiagCursor);
        break;
    case BOTTOMRIGHT:
        setCursor(Qt::SizeFDiagCursor);
        break;
    default:
        break;
    }
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

    switch (type) {
    case BOTTOM:
        g.setBottom(g.bottom() + dy);
        break;
    case LEFT:
        g.setLeft(g.left() + dx);
        break;
    case RIGHT:
        g.setRight(g.right() + dx);
        break;
    case BOTTOMLEFT:
        g.setBottom(g.bottom() + dy);
        g.setLeft(g.left() + dx);
        break;
    case BOTTOMRIGHT:
        g.setBottom(g.bottom() + dy);
        g.setRight(g.right() + dx);
    default:
        break;
    }
    topwindow->setGeometry(g);
    oldPos = e->globalPos();
}

void Border::leaveEvent(QEvent *)
{
    setCursor(Qt::ArrowCursor);
}
