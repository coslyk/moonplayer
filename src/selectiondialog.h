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
    QString showDialog(const QStringList &list, const QString &label, const QString &checkboxText = QString());
    bool isChecked(void);
    ~SelectionDialog();

private:
    Ui::SelectionDialog *ui;
};

#endif // SELECTIONDIALOG_H
