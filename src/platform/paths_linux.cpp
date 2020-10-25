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

#include "paths.h"
#include <QDir>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QStandardPaths>

QString userResourcesPath()
{
    static QString path;
    if (path.isNull())
    {
        const char *dataPath = getenv("XDG_DATA_HOME");
        path = dataPath ? QString::fromUtf8(dataPath) + QStringLiteral("/moonplayer") : QDir::homePath() + QStringLiteral("/.local/share/moonplayer");
    }
    return path;
}

// FFmpeg's file path
QString ffmpegFilePath()
{
    static QString filename;
    if (filename.isNull())
    {
        QString appPath = QCoreApplication::applicationDirPath();
        filename = appPath.endsWith(QStringLiteral("/bin")) ? appPath + QStringLiteral("/ffmpeg") : QStringLiteral("ffmpeg");
    }
    return filename;
}

// hlsdl's file path
QString hlsdlFilePath()
{
    static QString filename;
    if (filename.isNull())
    {
        QString appPath = QCoreApplication::applicationDirPath();
        filename = appPath.endsWith(QStringLiteral("/bin")) ? appPath + QStringLiteral("/moonplayer-hlsdl") : QStringLiteral("moonplayer-hlsdl");
    }
    return filename;
}
