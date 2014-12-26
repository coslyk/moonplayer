#ifndef MYBUTTONGROUP_H
#define MYBUTTONGROUP_H

#include <QWidget>
#include <QPushButton>

//******************
// MyButton
//*****************
class MyButton : public QPushButton
{
    Q_OBJECT
public:
    explicit MyButton(const QString &text, QWidget *parent = 0);
signals:
    void selected(MyButton *self);
private slots:
    void onClicked(void);
};


//******************
// MyButtonGroup
//*****************
class MyButtonGroup : public QWidget
{
    Q_OBJECT
public:
    explicit MyButtonGroup(QStringList list, QWidget *parent = 0);
    inline QString selectedText() {return selectedButton->text();}
signals:
    void selectedChanged(void);
private:
    MyButton *selectedButton;
private slots:
    void onNewSelected(MyButton *newButton);
};
#endif // MYBUTTONGROUP_H
