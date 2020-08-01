#include "paths.h"
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>

QString appResourcesPath()
{
    static QString path;
    if (path.isNull())
        path = QCoreApplication::applicationDirPath().replace(QStringLiteral("/MacOS"), QStringLiteral("/Resources"));
    return path;
}


QString userResourcesPath()
{
    static QString path;
    if (path.isNull())
        path = QDir::homePath() + QStringLiteral("/Library/Application Support/MoonPlayer");
    return path;
}

//get ffmpeg's file path
QString ffmpegFilePath()
{
    static QString filename;
    if (filename.isNull())
        filename = QCoreApplication::applicationDirPath() + QStringLiteral("/ffmpeg");
    return filename;
}
