#include "reslibrary.h"
#include "ui_reslibrary.h"
#include "resplugin.h"
#include "accessmanager.h"
#include "detailview.h"
#include "downloader.h"
#include "mybuttongroup.h"
#include "pyapi.h"
#include "mylistwidget.h"
#include <QMessageBox>
#include <QLabel>

//ResLibrary
ResLibrary *res_library = NULL;

ResLibrary::ResLibrary(QWidget *parent) :
    QWidget(parent), ui(new Ui::ResLibrary)
{
    printf("Initialize ResLibrary...\n");
    initResPlugins();
    ui->setupUi(this);
    listWidget = new MyListWidget;
    ui->gridLayout->addWidget(listWidget, 0, 1, 1, 4);
    ui->keyLineEdit->setFixedWidth(220);
    ui->stackedWidget->setFixedWidth(220);
    setMinimumSize(QSize(950, 650));

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

        layout->addWidget(new QLabel(tr("Regions:")));
        buttonGroup = new MyButtonGroup(plugin->countriesList);
        connect(buttonGroup, SIGNAL(selectedChanged()), this, SLOT(reSearch()));
        layout->addWidget(buttonGroup);
    }
    if (n_resplugins)
    {
        current_plugin = 0;
        current_tag = resplugins[0]->tagsList[0];
        current_country = resplugins[0]->countriesList[0];
        current_page = 1;
    }
    else
    {
        ui->keyLineEdit->setEnabled(false);
        ui->pageSpinBox->setEnabled(false);
        ui->nextPushButton->setEnabled(false);
        ui->prevPushButton->setEnabled(false);
    }
    res_library = this;
    detailView = NULL;

    ui->tabWidget->addTab(new Downloader, tr("Downloader"));

    connect(ui->pageSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onPageChanged(int)));
    connect(ui->prevPushButton, SIGNAL(clicked()), this, SLOT(onPrevPage()));
    connect(ui->nextPushButton, SIGNAL(clicked()), this, SLOT(onNextPage()));
    connect(ui->keyLineEdit, SIGNAL(returnPressed()), this, SLOT(keySearch()));
    connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(onItemDoubleClicked(QListWidgetItem*)));
}

void ResLibrary::reSearch()
{
    if (PluginIsLoadingPage())
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
    resplugins[current_plugin]->explore(current_tag, current_country, 1);
}

void ResLibrary::keySearch()
{
    if (PluginIsLoadingPage())
    {
        QMessageBox::warning(this, "warning", tr("Another file is parsing. Please wait."));
        return;
    }
    current_key = ui->keyLineEdit->text();
    current_plugin = ui->pluginComboBox->currentIndex();
    current_page = 1;
    ui->pageSpinBox->setValue(1);
    ui->prevPushButton->setEnabled(false);
    resplugins[current_plugin]->search(current_key, 1);
}

void ResLibrary::onItemDoubleClicked(QListWidgetItem *item)
{
    if (PluginIsLoadingPage())
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
    if (PluginIsLoadingPage())
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
            resplugins[current_plugin]->explore(current_tag, current_country, newPage);
        else
            resplugins[current_plugin]->search(current_key, newPage);
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

void ResLibrary::addItem(const QString &name, const QString &pic_url, const QString &flag)
{
    listWidget->addPicItem(name, pic_url, flag);
}

void ResLibrary::clearItem()
{
    listWidget->clearItem();
}

void ResLibrary::openDetailPage(const QVariantHash &data)
{
    if (detailView == NULL)
    {
        detailView = new DetailView;
        ui->tabWidget->addTab(detailView, "detail");
    }
    detailView->loadDetail(data);
    ui->tabWidget->setCurrentIndex(2);
    ui->tabWidget->setTabText(2, detailView->windowTitle());
}
