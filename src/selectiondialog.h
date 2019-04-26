#ifndef SELECTIONDIALOG_H
#define SELECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class SelectionDialog;
}

class SelectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SelectionDialog(QWidget *parent = 0);
    QString showDialog(const QStringList &list,
                       const QString &msg,
                       const QString &checkBoxText = QString(),
                       bool *isChecked = NULL);
    int showDialog_Index(const QStringList &list,
                         const QString &msg,
                         const QString &checkBoxText = QString(),
                         bool *isChecked = NULL);
    ~SelectionDialog();

private:
    Ui::SelectionDialog *ui;
};

#endif // SELECTIONDIALOG_H
