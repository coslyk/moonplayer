#ifndef BROWSER_H
#define BROWSER_H

#include <QWidget>

namespace Ui {
class Browser;
}
class QDialog;
class QLineEdit;

class Browser : public QWidget
{
    Q_OBJECT

public:
    explicit Browser(QWidget *parent = 0);
    ~Browser();

private slots:
    void openInputUrl(void);
    void onAddButton(void);
    void onRemoveButton(void);
    void onListItemClicked(const QModelIndex &);

private:
    Ui::Browser *ui;
    QDialog *addItemDialog;
    QLineEdit *titleInput;
    QLineEdit *urlInput;
    QHash<QString, QString> favorites;
};

#endif // BROWSER_H
