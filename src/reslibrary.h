#ifndef RESLIBRARY_H
#define RESLIBRARY_H

#include <QWidget>
#include <QList>
#include <QListWidgetItem>

namespace Ui {
class ResLibrary;
}
class QNetworkReply;

//******************
// ResListWidgetItem
//*****************
class ResListWidgetItem : public QListWidgetItem
{
public:
    ResListWidgetItem(const QString &name, const QByteArray &pic_url, const QByteArray &flag);
    inline QByteArray picUrl() {return m_picUurl;}
    inline QByteArray flag() {return m_flag;}
private:
    QByteArray m_picUurl;
    QByteArray m_flag;
};


//******************
// ResLibrary
//*****************
class ResLibrary : public QWidget
{
    Q_OBJECT
public:
    explicit ResLibrary(QWidget *parent = 0);
    void addItem(const QString &name, const QByteArray &pic_url, const QByteArray &flag);
    void clearItem(void);

private:
    Ui::ResLibrary *ui;
    int current_plugin;
    int current_page;
    QString current_tag;
    QString current_country;
    QString current_key;

    QList<ResListWidgetItem*> items_to_load_pic;
    QList<QByteArray> urls_for_load_pic;
    ResListWidgetItem *loading_item;
    QNetworkReply *reply;
    void loadNextPic(void);

private slots:
    void reSearch(void);
    void keySearch(void);
    void onItemDoubleClicked(QListWidgetItem *item);
    void onPageChanged(int newPage);
    void onPrevPage(void);
    void onNextPage(void);
    void onLoadPicFinished(void);
};

extern ResLibrary *res_library;

#endif // RESLIBRARY_H
