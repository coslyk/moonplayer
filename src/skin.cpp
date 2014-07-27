#include "skin.h"
#include <QWidget>
#include <QPixmap>
#include <QIcon>
#include <QPushButton>
#include <QMouseEvent>
#include <QFile>

void Skin::setWidgetBG(QWidget *widget, const QString& img)
{
    static QString sheet = "QWidget#%1{border-image:url(%2);}";

    if (!QFile::exists(img))
        widget->setStyleSheet(0);
    else
    {
        QString msg = sheet.arg(widget->objectName(), img);
        widget->setStyleSheet(msg);
    }
}

void Skin::setSlider(QWidget *slider, const QString &bg, const QString &handle)
{
    static QString sheet =
            "QSlider::groove:horizontal {"
            "    height: 5px;"
            "    margin: 0px 0px;"
            "    left: 12px; right: 12px;"
            "    border-image:url(%1);"
            "}"
            "QSlider::handle:horizontal {"
            "    border: 1px solid #5c5c5c;"
            "    border-image:url(%2);"
            "    width: 9px;"
            "    margin: -2px -2px -2px -2px;"
            "}";
    if (!QFile::exists(bg))
        slider->setStyleSheet(0);
    else
        slider->setStyleSheet(sheet.arg(bg, handle));
}

void Skin::setButton(QPushButton *button, const QIcon &icon, const QSize &size)
{
    button->setStyleSheet(0);
    button->setIcon(icon);
    button->setIconSize(size);
    button->setFixedSize(size);
    button->setFocusPolicy(Qt::NoFocus);
    button->setFlat(true);
}

void Skin::setButton(QPushButton *button, const QString &normal, const QString &hover, const QString &pressed)
{
    button->setIcon(QIcon());
    static QString stylesheet =
            "QPushButton {border-image:url(%1);}"
            "QPushButton:hover:!pressed {border-image:url(%2);}"
            "QPushButton:hover:pressed {border-image:url(%3);}";
    button->setText(0);
    QSize size = QPixmap(normal).size();
    button->setFixedSize(size);
    if (pressed.isNull())
        button->setStyleSheet(stylesheet.arg(normal, hover, normal));
    else
        button->setStyleSheet(stylesheet.arg(normal, hover, pressed));
    button->setFocusPolicy(Qt::NoFocus);
    button->setFlat(true);
}

void Skin::setListView(QWidget *listview, const QString &bg, const QString &selected,
                       const QString &inactive, const QString &scrollbar, const QString &scrollbg,
                       const QString &hscrollbar, const QString &hscrollbg)
{
    static QString stylesheet =
            "QListView {"
            "    alternate-background-color: white;"
            "    border-image: url(%1);"
            "}"
            "QListView::item:selected:active {border-image:url(%2);}"
            "QListView::item:selected:!active{border-image:url(%3);}"
            "QListView::item:hover {border: none;}"
            "QScrollBar::handle:vertical {"
            "    min-height: 20px;"
            "    margin: 0px;"
            "    border-radius: 3px;"
            "    border-image: url(%4);"
            "}"
            "QScrollBar:vertical {"
            "    border-image: url(%5);"
            "    width: 11px;"
            "    margin: 0px;"
            "}"
            "QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical {"
            "    border-image: url(%5);"
            "    height: 0px;"
            "    subcontrol-position: bottom;"
            "    subcontrol-origin: margin;"
            "}"
            "QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {"
            "    border-image: url(%5);"
            "}"
            "QScrollBar::handle:horizontal {"
            "    min-width: 20px;"
            "    margin: 0px;"
            "    border-radius: 3px;"
            "    border-image: url(%6);"
            "}"
            "QScrollBar:horizontal {"
            "    border-image: url(%7);"
            "    height: 11px;"
            "    margin: 0px;"
            "}"
            "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal {"
            "    border-image: url(%7);"
            "    width: 0px;"
            "    subcontrol-position: right;"
            "    subcontrol-origin: margin;"
            "}"
            "QScrollBar::add-page:horizontal, QScrollBar::sub-page:horizontal {"
            "    border-image: url(%7);"
            "}";
    if (QFile::exists(bg))
        listview->setStyleSheet(stylesheet.arg(bg, selected, inactive, scrollbar, scrollbg, hscrollbar, hscrollbg));
    else
        listview->setStyleSheet(0);
}

//window's borders
Border::Border(QWidget *topwin, BorderType t) :
    QWidget(0)
{
    type = t;
    topwindow = topwin;
    if (type == BOTTOM)
        setFixedHeight(4);
    else
        setFixedWidth(4);
}

void Border::setPicture(const QPixmap &img)
{
    if (img.isNull())
    {
        setAutoFillBackground(false);
        hide();
        return;
    }
    show();
    setAutoFillBackground(true);
    QPalette pal;
    pal.setBrush(backgroundRole(), QBrush(img));
    setPalette(pal);
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
