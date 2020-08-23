#include "utils.h"
#include <QCoreApplication>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegularExpression>
#include <QUrl>
#include "accessManager.h"
#include "console.h"


void Utils::checkUpdate()
{
    QUrl url(QStringLiteral("https://raw.githubusercontent.com/coslyk/moonplayer/develop/CMakeLists.txt"));
    QNetworkReply* reply = NetworkAccessManager::instance()->get(QNetworkRequest(url));
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        static QRegularExpression re(QStringLiteral("project\\(moonplayer VERSION (\\d+\\.\\d+)\\)"));
        if (reply->error() == QNetworkReply::NoError)
        {
            QString data = QString::fromLatin1(reply->readAll());
            QString latestVersion = re.match(data).captured(1);
            if (latestVersion != QStringLiteral(MOONPLAYER_VERSION))
                QMessageBox::information(nullptr, tr("Update"), tr("New version of MoonPlayer is available."));
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
    args << QStringLiteral("-ExecutionPolicy") << QStringLiteral("RemoteSigned");
    args << QStringLiteral("-File") << (QCoreApplication::applicationDirPath() + QStringLiteral("/update-parsers.ps1"));
    c_console->launchScript(QStringLiteral("powershell"), args);
#else
    static QString shell;
    if (shell.isNull())
    {
        QFile f(QStringLiteral(":/scripts/update-parsers.sh"));
        f.open(QFile::ReadOnly);
        shell = QString::fromLatin1(f.readAll());
    }
    args << QStringLiteral("-c") << shell;
    c_console->launchScript(QStringLiteral("sh"), args);
#endif
}


QString Utils::environmentVariable(const QString& env)
{
    return QString::fromUtf8(qgetenv(env.toUtf8().constData()));
}

