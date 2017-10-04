#include "skin.h"
#include "settings_player.h"
#include <QWidget>
#include <QMouseEvent>
#include <QRegularExpression>

QString scaleStyleSheet(const QString qss)
{
    static QRegularExpression re("(\\d+)px");
    QString newQss;
    QRegularExpressionMatch match;
    int offset = 0;
    match = re.match(qss, 0);
    while (match.hasMatch())
    {
        newQss += qss.mid(offset, match.capturedStart() - offset);
        newQss += QString::number((int) (match.captured(1).toInt() * Settings::uiScale));
        newQss += "px";
        offset = match.capturedEnd();
        match = re.match(qss, offset);
    }
    newQss += qss.mid(offset);
    return newQss;
}

//window's borders
Border::Border(QWidget *topwin, BorderType t) :
    QWidget(0)
{
    type = t;
    topwindow = topwin;
    switch (type) {
    case BOTTOM:
        setFixedHeight(4 * Settings::uiScale);
        break;
    case LEFT:
    case RIGHT:
        setFixedWidth(4 * Settings::uiScale);
        break;
    case BOTTOMLEFT:
    case BOTTOMRIGHT:
        setFixedSize(4 * Settings::uiScale, 4 * Settings::uiScale);
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
