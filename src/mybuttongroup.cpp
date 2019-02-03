#include "mybuttongroup.h"
#include <QGridLayout>

// MyButton
MyButton::MyButton(const QString &text, QWidget *parent) :
    QPushButton(text, parent)
{
    setFlat(true);
    setFocusPolicy(Qt::NoFocus);
    connect(this, SIGNAL(clicked()), this, SLOT(onClicked()));
}

void MyButton::onClicked()
{
    emit selected(this);
}

// MyButtonGroup
MyButtonGroup::MyButtonGroup(QStringList list, QWidget *parent) :
    QWidget(parent)
{
    selectedButton = nullptr;
    QGridLayout *layout = new QGridLayout(this);
    layout->setSpacing(0);
    int size = list.size();
    for (int i = 0; i < size; i++)
    {
        MyButton *button = new MyButton(list[i]);
        layout->addWidget(button, i/4, i%4);
        connect(button, SIGNAL(selected(MyButton*)), this, SLOT(onNewSelected(MyButton*)));
        // Set first button as selected button
        if (i == 0)
            selectedButton = button;
    }
    selectedButton->setDown(true);
    setFixedHeight((selectedButton->sizeHint().height() + 4) * layout->rowCount());
}

void MyButtonGroup::onNewSelected(MyButton *newButton)
{
    selectedButton->setDown(false);
    newButton->setDown(true);
    if (newButton != selectedButton)
    {
        selectedButton = newButton;
        emit selectedChanged();
    }
}
