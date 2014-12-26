#include "reslibrary.h"
#include "ui_reslibrary.h"
#include "resplugin.h"
#include "accessmanager.h"
#include "mybuttongroup.h"
#include "pyapi.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <iostream>

//ResListWidgetItem
ResListWidgetItem::ResListWidgetItem(const QString &name, const QByteArray &pic_url, const QByteArray &flag) :
    QListWidgetItem(name)
{
    m_picUurl = pic_url;
    m_flag = flag;
}


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
    for (int i = 0; i < n_resplugins; i++)
    {
        ResPlugin *plugin = resplugins[i];
        ui->pluginComboBox->addItem(plugin->getName());
        MyButtonGroup *buttonGroup = new MyButtonGroup(plugin->tagsList);
        connect(buttonGroup, SIGNAL(selectedChanged()), this, SLOT(reSearch()));
        ui->tagStackedWidget->addWidget(buttonGroup);
        buttonGroup = new MyButtonGroup(plugin->countriesList);
        connect(buttonGroup, SIGNAL(selectedChanged()), this, SLOT(reSearch()));
        ui->countryStackedWidget->addWidget(buttonGroup);
    }
    current_plugin = 0;
    current_tag = resplugins[0]->tagsList[0];
    current_country = resplugins[0]->countriesList[0];
    current_page = 1;
    loading_item = NULL;
    reply = NULL;
    res_library = this;

    connect(ui->pageSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPageChanged(int)));
    connect(ui->prevPushButton, SIGNAL(clicked()), this, SLOT(onPrevPage()));
    connect(ui->nextPushButton, SIGNAL(clicked()), this, SLOT(onNextPage()));
    connect(ui->keyLineEdit, SIGNAL(returnPressed()), this, SLOT(keySearch()));
    connect(ui->listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked(QListWidgetItem*)));
}

void ResLibrary::reSearch()
{
    if (geturl_obj->hasTask())
    {
        QMessageBox::warning(this, "warning", tr("Another file is parsing. Please wait."));
        return;
    }
    MyButtonGroup *tagsButtonGroup = static_cast<MyButtonGroup*>(ui->tagStackedWidget->currentWidget());
    MyButtonGroup *countriesButtonGroup = static_cast<MyButtonGroup*>(ui->countryStackedWidget->currentWidget());
    current_tag = tagsButtonGroup->selectedText();
    current_country = countriesButtonGroup->selectedText();
    current_plugin = ui->tagStackedWidget->currentIndex();
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
    ResListWidgetItem *res_item = static_cast<ResListWidgetItem*>(item);
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
    ResListWidgetItem *item = new ResListWidgetItem(name, pic_url, flag);
    items_to_load_pic << item;
    if (loading_item == NULL)
        loadNextPic();
}

void ResLibrary::loadNextPic()
{
    loading_item = items_to_load_pic.takeFirst();
    QNetworkRequest request(QString::fromUtf8(loading_item->picUrl()));
    request.setRawHeader("User-Agent", "moonplayer");
    reply = access_manager->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onLoadPicFinished()));
}

void ResLibrary::onLoadPicFinished()
{
    if (loading_item)
    {
        QPixmap pic;
        pic.loadFromData(reply->readAll());
        if (pic.width() > 100)
            pic = pic.scaledToWidth(100, Qt::SmoothTransformation);
        loading_item->setIcon(QIcon(pic));
        loading_item->setSizeHint(pic.size() + QSize(10, 20));
        ui->listWidget->setIconSize(pic.size());
        ui->listWidget->addItem(loading_item);
    }
    reply->deleteLater();
    reply = NULL;
    if (items_to_load_pic.size())
        loadNextPic();
    else
        loading_item = NULL;
}

void ResLibrary::clearItem()
{
    loading_item = NULL;
    QList<ResListWidgetItem*> items_to_clear = items_to_load_pic;
    items_to_load_pic.clear();
    ui->listWidget->clear();
    while (!items_to_clear.isEmpty())
    {
        ResListWidgetItem *item = items_to_clear.takeFirst();
        delete item;
    }
}
