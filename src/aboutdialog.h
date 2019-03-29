#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

namespace Ui {
class AboutDialog;
}
class QNetworkReply;

class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = NULL);
    ~AboutDialog();

private:
    Ui::AboutDialog *ui;
    QNetworkReply *reply;

private slots:
    void checkUpdateFinished(void);
};

#endif // ABOUTDIALOG_H
