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
    MyListWidgetItem(const QString &name, const QByteArray &pic_url, const QByteArray &flag);
    inline QByteArray picUrl() {return m_picUrl;}
    inline QByteArray flag() {return m_flag;}
private:
    QByteArray m_picUrl;
    QByteArray m_flag;
};

//******************
// MyListWidget
//*****************
class MyListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit MyListWidget(QWidget *parent = 0);
    void addPicItem(const QString &name, const QByteArray &picUrl, const QByteArray &flag = QByteArray());
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
