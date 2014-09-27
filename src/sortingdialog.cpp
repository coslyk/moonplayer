#include "sortingdialog.h"
#include "ui_sortingdialog.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>

SortingDialog::SortingDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SortingDialog)
{
    ui->setupUi(this);
    connect(ui->addPushButton, SIGNAL(clicked()), this, SLOT(onAddButton()));
    connect(ui->delPushButton, SIGNAL(clicked()), this, SLOT(onDelButton()));
    connect(ui->upPushButton, SIGNAL(clicked()), this, SLOT(onUpButton()));
    connect(ui->downPushButton, SIGNAL(clicked()), this, SLOT(onDownButton()));
    connect(ui->saveToPushButton, SIGNAL(clicked()), this, SLOT(onSaveToButton()));
}

SortingDialog::~SortingDialog()
{
    delete ui;
}

void SortingDialog::execDialog(QStringList &list, const QString &saveTo)
{
    // Load files to QListWidget
    ui->listWidget->clear();
    ui->listWidget->addItems(list);
    // Set file to save to
    if (saveTo.isEmpty())
        save_to = QDir::home().filePath(QFileInfo(list[0]).fileName());
    else
        save_to = saveTo;
    ui->saveToLabel->setText(tr("Save to:") + save_to);
    exec();
    // Save changes to list
    list.clear();
    for (int i = 0; i < ui->listWidget->count(); i++)
        list.append(ui->listWidget->item(i)->text());
}

void SortingDialog::onAddButton()
{
    QStringList files = QFileDialog::getOpenFileNames(this, "Choose files");
    ui->listWidget->addItems(files);
}

void SortingDialog::onDelButton()
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (item)
        delete item;
}

void SortingDialog::onUpButton()
{
    int row = ui->listWidget->currentRow();
    if (row == -1 || row == 0)
        return;
    QListWidgetItem *item = ui->listWidget->takeItem(row);
    ui->listWidget->insertItem(row - 1, item);
    ui->listWidget->setCurrentItem(item);
}

void SortingDialog::onDownButton()
{
    int row = ui->listWidget->currentRow();
    if (row == -1 || row == ui->listWidget->count() - 1)
        return;
    QListWidgetItem *item = ui->listWidget->takeItem(row);
    ui->listWidget->insertItem(row + 1, item);
    ui->listWidget->setCurrentItem(item);
}

void SortingDialog::onSaveToButton()
{
    QString file = QFileDialog::getSaveFileName(this, tr("Set file name"), save_to);
    if (!file.isEmpty())
    {
        save_to = file;
        if (!QFileInfo(save_to).fileName().contains('.'))
            save_to += ".mp4";
        ui->saveToLabel->setText(tr("Save to:") + save_to);
    }
}
