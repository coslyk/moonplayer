#include "browser.h"
#include "ui_browser.h"

Browser::Browser(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Browser)
{
    ui->setupUi(this);
    connect(ui->urlEdit, &QLineEdit::returnPressed, this, &Browser::openInputUrl);
    connect(ui->goButton, &QPushButton::clicked, this, &Browser::openInputUrl);
}

Browser::~Browser()
{
    delete ui;
}

void Browser::openInputUrl()
{
    ui->webEngineView->setUrl(ui->urlEdit->text());
}
