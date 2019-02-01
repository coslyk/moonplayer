#ifndef DETAILVIEW_H
#define DETAILVIEW_H

#include <QWidget>

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
    void loadDetail(const QVariantHash &data);

private:
    Ui::DetailView *ui;
    QNetworkReply *reply;
    QStringList urls;

private slots:
    void onImageLoaded(void);
    void onPlay(void);
    void onDownload(void);
};

#endif // DETAILVIEW_H
