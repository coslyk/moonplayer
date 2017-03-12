#include "selectiondialog.h"
#include "ui_selectiondialog.h"

SelectionDialog::SelectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SelectionDialog)
{
    ui->setupUi(this);
}

QString SelectionDialog::showDialog(const QStringList &list, const QString &label, const QString &checkboxText)
{
    ui->listWidget->clear();
    foreach (QString item, list) {
        ui->listWidget->addItem(item);
    }
    ui->label->setText(label);
    if (checkboxText.isEmpty())
        ui->checkBox->hide();
    else
    {
        ui->checkBox->setText(checkboxText);
        ui->checkBox->show();
    }

    int state = exec();
    if (state == QDialog::Accepted && ui->listWidget->currentItem())
        return ui->listWidget->currentItem()->text();
    else
        return QString();
}

bool SelectionDialog::isChecked()
{
    return ui->checkBox->isChecked();
}

SelectionDialog::~SelectionDialog()
{
    delete ui;
}
