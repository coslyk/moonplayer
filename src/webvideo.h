#ifndef WEBVIDEO_H
#define WEBVIDEO_H

#include <QTabWidget>
#include <Python.h>
class QListWidgetItem;
class QColor;
class Downloader;
class QListWidget;
class QPushButton;
class QComboBox;
class QLineEdit;

//Get videos from Video websites
class WebVideo : public QTabWidget
{
    Q_OBJECT
    
signals:
    void newUrl(const QString& name, const QString& url);
    void newUrlAndPlay(const QString& name, const QString& url);

public:
    explicit WebVideo(QWidget *parent = 0);
    PyObject* showList(PyObject *list);
    void setListItemColor(int n, const QColor &color);
    QListWidget *listWidget;

protected:
    void showEvent(QShowEvent *);

private:
    QVector<QByteArray> result; //search result
    QString keyword;
    int page_n;
    int provider;
    void warnHavingTask(void);
    void startPlayOrDown(QListWidgetItem* item);

    QComboBox *comboBox;
    QPushButton *prevButton;
    QPushButton *nextButton;
    QLineEdit *lineEdit;

private slots:
    void downSearchPage(void);
    void nextSearchPage(void);
    void prevSearchPage(void);
    void searchVideo(void);
    void onDoubleClicked(QListWidgetItem *item);
    void onDownButton(void);
};
extern WebVideo *webvideo;

#endif // WEBVIDEO_H
