#ifndef MYLISTWIDGET_H
#define MYLISTWIDGET_H

#include <QListWidget>
class QNetworkReply;

//******************
// MyListWidgetItem
//*****************
class MyListWidgetItem : public QListWidgetItem
{
public:
    MyListWidgetItem(const QString &name, const QString &pic_url, const QString &flag);
    inline QString picUrl() {return m_picUrl;}
    inline QString flag() {return m_flag;}
private:
    QString m_picUrl;
    QString m_flag;
};

//******************
// MyListWidget
//*****************
class MyListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit MyListWidget(QWidget *parent = 0);
    void addPicItem(const QString &name, const QString &picUrl, const QString &flag = QString());
    void clearItem(void);

private:
    QList<MyListWidgetItem*> items_to_load_pic;
    MyListWidgetItem *loading_item;
    QNetworkReply *reply;
    void loadNextPic(void);

private slots:
    void onLoadPicFinished(void);
};

#endif // MYLISTWIDGET_H
