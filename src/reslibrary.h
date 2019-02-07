#ifndef RESLIBRARY_H
#define RESLIBRARY_H

#include <QWidget>
#include <QListWidgetItem>

namespace Ui {
class ResLibrary;
}
class MyListWidget;
class DetailView;

//******************
// ResLibrary
//*****************
class ResLibrary : public QWidget
{
    Q_OBJECT
public:
    explicit ResLibrary(QWidget *parent = 0);
    void addItem(const QString &name, const QString &pic_url, const QString &flag);
    void clearItem(void);
    void openDetailPage(const QVariantHash &data);

private:
    Ui::ResLibrary *ui;
    int current_plugin;
    int current_page;
    QString current_tag;
    QString current_country;
    QString current_key;
    MyListWidget *listWidget;
    DetailView *detailView;

private slots:
    void reSearch(void);
    void keySearch(void);
    void onItemDoubleClicked(QListWidgetItem *item);
    void onPageChanged(int newPage);
    void onPrevPage(void);
    void onNextPage(void);
};

extern ResLibrary *res_library;

#endif // RESLIBRARY_H
