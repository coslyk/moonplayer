#include "updatechecker.h"
#include "accessmanager.h"
#include "settings_player.h"
#include <QFile>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

UpdateChecker::UpdateChecker(QObject *parent) : QObject(parent)
{

}

void UpdateChecker::check()
{
    QUrl url("https://raw.githubusercontent.com/coslyk/moonplayer/master/src/Version");
    reply = access_manager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, &UpdateChecker::onFinished);
}

void UpdateChecker::onFinished()
{
    if (reply->error() == QNetworkReply::NoError)
    {
        int newVersion = reply->readAll().simplified().toInt();
        QFile file(Settings::path + "/Version");
        if (file.open(QFile::ReadOnly))
        {
            int currentVersion = file.readAll().simplified().toInt();
            file.close();
            if (newVersion > currentVersion)
                QMessageBox::warning(NULL, "Moon Player", tr("New version is available."));
        }
    }
    reply->deleteLater();
}
