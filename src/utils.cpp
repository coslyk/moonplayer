/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

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

