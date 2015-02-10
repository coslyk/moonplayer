#include "reslibrary.h"
#include "ui_reslibrary.h"
#include "resplugin.h"
#include "accessmanager.h"
#include "mybuttongroup.h"
#include "pyapi.h"
#include "mylistwidget.h"
#include <QMessageBox>
#include <QLabel>
#include <iostream>


//ResLibrary
ResLibrary *res_library = NULL;

ResLibrary::ResLibrary(QWidget *parent) :
    QWidget(parent), ui(new Ui::ResLibrary)
{
    std::cout << "Initialize ResLibrary..." << std::endl;
    initResPlugins();
    if (n_resplugins == 0)
    {
        new QLabel("This function is under development. Please wait!", this);
        return;
    }
    ui->setupUi(this);
    listWidget = new MyListWidget;
    ui->gridLayout->addWidget(listWidget, 0, 1, 1, 4);

    for (int i = 0; i < n_resplugins; i++)
    {
        ResPlugin *plugin = resplugins[i];
        ui->pluginComboBox->addItem(plugin->getName());

        QWidget *page = new QWidget;
        ui->stackedWidget->addWidget(page);
        QBoxLayout *layout = new QBoxLayout(QBoxLayout::TopToBottom);
        layout->setContentsMargins(0, 11, 0, 11);
        page->setLayout(layout);

        layout->addWidget(new QLabel(tr("Tags:")));
        MyButtonGroup *buttonGroup = new MyButtonGroup(plugin->tagsList);
        connect(buttonGroup, SIGNAL(selectedChanged()), this, SLOT(reSearch()));
        layout->addWidget(buttonGroup);

        layout->addWidget(new QLabel(tr("Countries:")));
        buttonGroup = new MyButtonGroup(plugin->countriesList);
        connect(buttonGroup, SIGNAL(selectedChanged()), this, SLOT(reSearch()));
        layout->addWidget(buttonGroup);
    }
    current_plugin = 0;
    current_tag = resplugins[0]->tagsList[0];
    current_country = resplugins[0]->countriesList[0];
    current_page = 1;
    res_library = this;

    connect(ui->pageSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPageChanged(int)));
    connect(ui->prevPushButton, SIGNAL(clicked()), this, SLOT(onPrevPage()));
    connect(ui->nextPushButton, SIGNAL(clicked()), this, SLOT(onNextPage()));
    connect(ui->keyLineEdit, SIGNAL(returnPressed()), this, SLOT(keySearch()));
    connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked(QListWidgetItem*)));
}

void ResLibrary::reSearch()
{
    if (geturl_obj->hasTask())
    {
        QMessageBox::warning(this, "warning", tr("Another file is parsing. Please wait."));
        return;
    }
    QList<MyButtonGroup*> groups = ui->stackedWidget->currentWidget()->findChildren<MyButtonGroup*>();
    current_tag = groups[0]->selectedText();
    current_country = groups[1]->selectedText();
    current_plugin = ui->stackedWidget->currentIndex();
    current_key = QString();
    current_page = 1;
    ui->pageSpinBox->setValue(1);
    ui->prevPushButton->setEnabled(false);
    resplugins[current_plugin]->search(current_tag, current_country, 1);
}

void ResLibrary::keySearch()
{
    if (geturl_obj->hasTask())
    {
        QMessageBox::warning(this, "warning", tr("Another file is parsing. Please wait."));
        return;
    }
    current_key = ui->keyLineEdit->text();
    current_plugin = ui->pluginComboBox->currentIndex();
    current_page = 1;
    ui->pageSpinBox->setValue(1);
    ui->prevPushButton->setEnabled(false);
    resplugins[current_plugin]->searchByKey(current_key, 1);
}

void ResLibrary::onItemDoubleClicked(QListWidgetItem *item)
{
    if (geturl_obj->hasTask())
    {
        QMessageBox::warning(this, "warning", tr("Another file is parsing. Please wait."));
        return;
    }
    MyListWidgetItem *res_item = static_cast<MyListWidgetItem*>(item);
    resplugins[current_plugin]->loadItem(res_item->flag());
}

// Set page
void ResLibrary::onPageChanged(int newPage)
{
    if (geturl_obj->hasTask())
    {
        QMessageBox::warning(this, "warning", tr("Another file is parsing. Please wait."));
        return;
    }
    if (newPage != current_page)
    {
        current_page = newPage;
        if (newPage == 1)
            ui->prevPushButton->setEnabled(false);
        else
            ui->prevPushButton->setEnabled(true);
        if (current_key.isEmpty())
            resplugins[current_plugin]->search(current_tag, current_country, newPage);
        else
            resplugins[current_plugin]->searchByKey(current_key, newPage);
    }
}

void ResLibrary::onPrevPage()
{
    ui->pageSpinBox->setValue(current_page - 1);
}

void ResLibrary::onNextPage()
{
    ui->pageSpinBox->setValue(current_page + 1);
}

void ResLibrary::addItem(const QString &name, const QByteArray &pic_url, const QByteArray &flag)
{
    listWidget->addPicItem(name, pic_url, flag);
}

void ResLibrary::clearItem()
{
    listWidget->clearItem();
}
