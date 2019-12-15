#include "utils.h"
#include "accessManager.h"
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QUrl>

void Utils::checkUpdate()
{
    QUrl url("https://raw.githubusercontent.com/coslyk/moonplayer/develop/CMakeLists.txt");
    QNetworkReply* reply = NetworkAccessManager::instance()->get(QNetworkRequest(url));
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        static QRegularExpression re("project\\(moonplayer VERSION (\\d+\\.\\d+)\\)");
        if (reply->error() == QNetworkReply::NoError)
        {
            QString data = reply->readAll();
            QString latestVersion = re.match(data).captured(1);
            if (latestVersion != MOONPLAYER_VERSION)
                QMessageBox::information(nullptr, "Update", "New version of MoonPlayer is available.");
        }
        reply->deleteLater();
    });
}

