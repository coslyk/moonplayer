#include "selectionDialog.h"
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QListWidget>
#include <QVBoxLayout>

SelectionDialog::SelectionDialog(QWidget *parent) :
    QDialog(parent)
{
    resize(400, 300);
    QVBoxLayout *verticalLayout = new QVBoxLayout(this);
    m_label = new QLabel;
    verticalLayout->addWidget(m_label);

    m_listWidget = new QListWidget;
    verticalLayout->addWidget(m_listWidget);

    m_checkBox = new QCheckBox;
    verticalLayout->addWidget(m_checkBox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox;
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
    verticalLayout->addWidget(buttonBox);
    
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    connect(m_listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(accept()));
}

QString SelectionDialog::showDialog(const QStringList &list,
                                    const QString &msg,
                                    const QString &checkBoxText,
                                    bool *isChecked)
{
    m_listWidget->clear();
    foreach (QString item, list) {
        m_listWidget->addItem(item);
    }
    m_label->setText(msg);
    if (checkBoxText.isEmpty())
        m_checkBox->hide();
    else
    {
        m_checkBox->setText(checkBoxText);
        m_checkBox->setChecked(false);
        m_checkBox->show();
    }

    int state = exec();
    if (state == QDialog::Accepted && m_listWidget->currentItem())
    {
        if (isChecked)
            *isChecked = m_checkBox->isChecked();
        return m_listWidget->currentItem()->text();
    }
    else
        return QString();
}

int SelectionDialog::showDialog_Index(const QStringList &list,
                                      const QString &msg,
                                      const QString &checkBoxText,
                                      bool *isChecked)
{
    m_listWidget->clear();
    foreach (QString item, list) {
        m_listWidget->addItem(item);
    }
    m_label->setText(msg);
    if (checkBoxText.isEmpty())
        m_checkBox->hide();
    else
    {
        m_checkBox->setText(checkBoxText);
        m_checkBox->setChecked(false);
        m_checkBox->show();
    }

    int state = exec();
    if (state == QDialog::Accepted && m_listWidget->currentItem())
    {
        if (isChecked)
            *isChecked = m_checkBox->isChecked();
        return m_listWidget->currentRow();
    }
    else
        return -1;
}


SelectionDialog::~SelectionDialog()
{
}
