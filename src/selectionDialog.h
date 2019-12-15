#ifndef SELECTIONDIALOG_H
#define SELECTIONDIALOG_H

#include <QDialog>

class QCheckBox;
class QLabel;
class QListWidget;

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
    QCheckBox *m_checkBox;
    QLabel *m_label;
    QListWidget *m_listWidget;
};

#endif // SELECTIONDIALOG_H
