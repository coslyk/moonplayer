#ifndef SORTINGDIALOG_H
#define SORTINGDIALOG_H

#include <QDialog>

namespace Ui {
class SortingDialog;
}

class SortingDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SortingDialog(QWidget *parent = 0);
    ~SortingDialog();
    void execDialog(QStringList &list, const QString &saveTo = QString());
    inline QString &saveTo(void) {return save_to;}

private:
    Ui::SortingDialog *ui;
    QString save_to;

private slots:
    void onAddButton(void);
    void onDelButton(void);
    void onUpButton(void);
    void onDownButton(void);
    void onSaveToButton(void);
};

#endif // SORTINGDIALOG_H
