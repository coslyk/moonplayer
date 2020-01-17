#include "utils.h"
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QUrl>
#include "accessManager.h"
#include "console.h"
#include "platform/paths.h"


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



void Utils::updateParser()
{
    static Console* c_console = nullptr;
    if (c_console == nullptr)
        c_console = new Console;
    QStringList args;
#ifdef Q_OS_WIN
    args << "-ExecutionPolicy" << "RemoteSigned";
    args << "-File" << (appResourcesPath() + "/update-parsers.ps1");
    c_console->launchScript("powershell", args);
#else
    args << appResourcesPath() + "/update-parsers.sh";
    c_console->launchScript("sh", args);
#endif
}

