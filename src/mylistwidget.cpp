#include "mylistwidget.h"
#include "accessmanager.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QPixmap>

//MyListWidgetItem
MyListWidgetItem::MyListWidgetItem(const QString &name, const QString &pic_url, const QString &flag) :
    QListWidgetItem(name)
{
    m_picUrl = pic_url;
    m_flag = flag;
}

MyListWidget::MyListWidget(QWidget *parent) :
    QListWidget(parent)
{
    setViewMode(QListWidget::IconMode);
    setSpacing(4);
    setResizeMode(QListWidget::Adjust);
    setWrapping(true);
    setMovement(QListWidget::Static);
    loading_item = nullptr;
    reply = nullptr;
}

void MyListWidget::addPicItem(const QString &name, const QString &picUrl, const QString &flag)
{
    MyListWidgetItem *item = new MyListWidgetItem(name, picUrl, flag);
    item->setToolTip(name);
    items_to_load_pic << item;
    if (loading_item == nullptr)
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
    if (loading_item)
    {
        QPixmap pic;
        pic.loadFromData(reply->readAll());
        if (pic.width())
            pic = pic.scaledToWidth(100, Qt::SmoothTransformation);
        loading_item->setIcon(QIcon(pic));
        loading_item->setSizeHint(pic.size() + QSize(10, 20));
        if (count() == 0) // First item
            setIconSize(pic.size());
        addItem(loading_item);
    }
    reply->deleteLater();
    reply = nullptr;
    if (items_to_load_pic.size())
        loadNextPic();
    else
        loading_item = nullptr;
}

void MyListWidget::clearItem()
{
    loading_item = nullptr;
    QList<MyListWidgetItem*> items_to_clear = items_to_load_pic;
    items_to_load_pic.clear();
    clear();
    while (!items_to_clear.isEmpty())
    {
        MyListWidgetItem *item = items_to_clear.takeFirst();
        delete item;
    }
}
