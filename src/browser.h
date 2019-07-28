#ifndef BROWSER_H
#define BROWSER_H

#include <QWidget>

namespace Ui {
class Browser;
}

class Browser : public QWidget
{
    Q_OBJECT

public:
    explicit Browser(QWidget *parent = 0);
    ~Browser();

private slots:
    void openInputUrl(void);

private:
    Ui::Browser *ui;
};

#endif // BROWSER_H
