#include "browser.h"
#include "ui_browser.h"
#include <QDialog>
#include <QDialogButtonBox>
#include <QMessageBox>
#include "utils.h"

Browser::Browser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Browser)
{
    ui->setupUi(this);
    connect(ui->urlEdit, &QLineEdit::returnPressed, this, &Browser::openInputUrl);
    connect(ui->goButton, &QPushButton::clicked, this, &Browser::openInputUrl);
    connect(ui->addButton, &QPushButton::clicked, this, &Browser::onAddButton);
    connect(ui->removeButton, &QPushButton::clicked, this, &Browser::onRemoveButton);
    connect(ui->listWidget, &QListWidget::clicked, this, &Browser::onListItemClicked);

    // Init add item dialog
    addItemDialog = new QDialog(this);
    QVBoxLayout *layout = new QVBoxLayout(addItemDialog);

    QLabel *titleLabel = new QLabel(tr("Title:"), this);
    titleInput = new QLineEdit(this);

    QLabel *urlLabel = new QLabel("URL:");
    urlInput = new QLineEdit(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttonBox, &QDialogButtonBox::accepted, addItemDialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, addItemDialog, &QDialog::reject);

    layout->addWidget(titleLabel);
    layout->addWidget(titleInput);
    layout->addWidget(urlLabel);
    layout->addWidget(urlInput);
    layout->addWidget(buttonBox);

    // load favorites
    favorites = loadQHashFromFile("favorite_urls.txt");

    //default favorites
    if (favorites.isEmpty())
    {
        favorites["Login to Youtube"] = "https://accounts.google.com/signin";
        favorites["Login to Youku"] = "https://account.youku.com/login.htm";
        favorites["Login to Bilibili"] = "https://passport.bilibili.com/login";
        saveQHashToFile(favorites, "favorite_urls.txt");
    }

    for (QHash<QString, QString>::const_iterator i = favorites.constBegin(); i != favorites.constEnd(); i++)
        ui->listWidget->addItem(i.key());
}

Browser::~Browser()
{
    delete ui;
}

void Browser::openInputUrl()
{
    ui->webEngineView->setUrl(ui->urlEdit->text());
}

void Browser::onAddButton()
{
    if (addItemDialog->exec() == QDialog::Accepted)
    {
        QString title = titleInput->text();
        QString url = urlInput->text();
        favorites[title] = url;
        ui->listWidget->addItem(title);
        saveQHashToFile(favorites, "favorite_urls.txt");
    }
}

void Browser::onRemoveButton()
{
    QListWidgetItem *currentItem = ui->listWidget->currentItem();
    if (currentItem)
    {
        QString title = currentItem->text();
        if (QMessageBox::warning(this,
                                 "Warning",
                                 tr("Are you sure to remove \"%1\"?").arg(title),
                                 QMessageBox::Yes,
                                 QMessageBox::No) == QMessageBox::No)
            return;
        delete currentItem;
        favorites.remove(title);
        saveQHashToFile(favorites, "favorite_urls.txt");
    }
}

void Browser::onListItemClicked(const QModelIndex &)
{
    QString title = ui->listWidget->currentItem()->text();
    ui->webEngineView->setUrl(favorites[title]);
}
