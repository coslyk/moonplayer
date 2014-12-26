#ifndef DETAILVIEW_H
#define DETAILVIEW_H

#include <QWidget>
#include <Python.h>
#include <QVector>

namespace Ui {
class DetailView;
}
class QNetworkReply;

class DetailView : public QWidget
{
    Q_OBJECT

public:
    explicit DetailView(QWidget *parent = 0);
    ~DetailView();
    PyObject *loadDetail(PyObject *dict);

private:
    Ui::DetailView *ui;
    QNetworkReply *reply;
    QVector<QByteArray> urls;

private slots:
    void onImageLoaded(void);
    void onPlay(void);
    void onDownload(void);
};

#endif // DETAILVIEW_H
