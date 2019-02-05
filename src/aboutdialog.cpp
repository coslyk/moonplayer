#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "accessmanager.h"
#include "parserbase.h"
#include "platform/paths.h"
#include <QFile>
#include <QMessageBox>
#include <QNetworkReply>

static const int currentVersion =
        #include "Version"
        ;
static int latestVersion = 0;

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    ui->setupUi(this);
    ui->currentVersion->setText(QString::number(currentVersion / 100.0));

    // check update
    QUrl url("https://raw.githubusercontent.com/coslyk/moonplayer/master/src/Version");
    reply = access_manager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &AboutDialog::checkUpdateFinished);
}

AboutDialog::~AboutDialog()
{
    delete ui;
}


void AboutDialog::checkUpdateFinished()
{
    if (reply->error() == QNetworkReply::NoError)
    {
        latestVersion = reply->readAll().simplified().toInt();
        ui->latestVersion->setText(QString::number(latestVersion / 100.0));
        if (latestVersion > currentVersion)
            QMessageBox::warning(nullptr, "Moon Player", tr("New version is available."));
    }
    reply->deleteLater();

    // first time to use MoonPlayer?
    if (!QFile::exists(getUserPath() + "/plugins-version.txt"))
    {
        QMessageBox::StandardButton btn = QMessageBox::information(nullptr,
                                                                   tr("Download plugins"),
                                                                   tr("No plugins founded. Download it now?"),
                                                                   QMessageBox::Yes,
                                                                   QMessageBox::No);
        if (btn == QMessageBox::Yes)
        {
            upgradeParsers();
            QMessageBox::information(nullptr, "Finish", tr("Downloading plugins finished. Please relaunch MoonPlayer."));
        }
    }
}
