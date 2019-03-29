#include "mylistwidget.h"
#include "accessmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>

//MyListWidgetItem
MyListWidgetItem::MyListWidgetItem(const QString &name, const QString &pic_url, const QString &flag) :
    QListWidgetItem(name)
{
    QPixmap pic(100, 150);
    pic.fill(QColor(200, 200, 200));
    m_picUrl = pic_url;
    m_flag = flag;
    setIcon(pic);
    setSizeHint(QSize(110, 170));
}

MyListWidget::MyListWidget(QWidget *parent) :
    QListWidget(parent)
{
    setViewMode(QListWidget::IconMode);
    setSpacing(4);
    setResizeMode(QListWidget::Adjust);
    setWrapping(true);
    setMovement(QListWidget::Static);
    setIconSize(QSize(100, 150));
    loading_item = NULL;
    reply = NULL;
}

void MyListWidget::addPicItem(const QString &name, const QString &picUrl, const QString &flag)
{
    MyListWidgetItem *item = new MyListWidgetItem(name, picUrl, flag);
    item->setToolTip(name);
    addItem(item);
    items_to_load_pic << item;
    if (loading_item == NULL)
        loadNextPic();
}

void MyListWidget::loadNextPic()
{
    loading_item = items_to_load_pic.takeFirst();
    QNetworkRequest request(loading_item->picUrl());
    reply = access_manager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadPicFinished()));
}

void MyListWidget::onLoadPicFinished()
{
    if (reply == NULL)
        return;

    if (loading_item)
    {
        QPixmap pic;
        pic.loadFromData(reply->readAll());
        if (!pic.isNull())
        {
            pic = pic.scaledToWidth(100, Qt::SmoothTransformation);
            loading_item->setIcon(QIcon(pic));
        }
    }
    reply->deleteLater();
    reply = NULL;
    if (items_to_load_pic.size())
        loadNextPic();
    else
        loading_item = NULL;
}

void MyListWidget::clearItem()
{
    if (reply)
    {
        reply->abort();
        reply->deleteLater();
        reply = NULL;
    }
    loading_item = NULL;
    items_to_load_pic.clear();
    clear();
}
