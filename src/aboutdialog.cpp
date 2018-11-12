#include "aboutdialog.h"
#include "ui_aboutdialog.h"
#include "accessmanager.h"
#include "platforms.h"
#include <QDir>
#include <QMessageBox>
#include <QNetworkAccessManager>
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
            QMessageBox::warning(NULL, "Moon Player", tr("New version is available."));
    }
    reply->deleteLater();

    // Move plugins to new path
#ifdef Q_OS_LINUX
    QDir dir(QDir::homePath() + "/.moonplayer/plugins");
    if (dir.entryList(QDir::Files).count())
    {
        QString cmd = QString("mv '%1/'* '%2/plugins'").arg(dir.path(), getUserPath());
        system(cmd.toUtf8().constData());
        QMessageBox::information(NULL, "Plugins moved",
                                 tr("Some plugins are found in ~/.moonplayer/plugins. "
                                    "This directory is no longer used, "
                                    "and all the plugins are moved to the new directory:"
                                    "\n\n%1/plugins/\n\n"
                                    "Please restart MoonPlayer to apply this change.").arg(getUserPath()));
    }
#endif
}
