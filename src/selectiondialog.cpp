#include "selectiondialog.h"
#include "ui_selectiondialog.h"

SelectionDialog::SelectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectionDialog)
{
    ui->setupUi(this);
}

void SelectionDialog::setList(const QStringList &list)
{
    ui->listWidget->clear();
    foreach (QString item, list) {
        ui->listWidget->addItem(item);
    }
}

bool SelectionDialog::remember()
{
    return ui->rememberCheckBox->isChecked();
}

QString SelectionDialog::selectedItem()
{
    QListWidgetItem *item = ui->listWidget->currentItem();
    if (item)
        return item->text();
    else
        return QString();
}

SelectionDialog::~SelectionDialog()
{
    delete ui;
}
