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
    void setList(const QStringList &list);
    bool remember(void);
    QString selectedItem(void);
    ~SelectionDialog();

private:
    Ui::SelectionDialog *ui;
};

#endif // SELECTIONDIALOG_H
