#include "paths.h"
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>

QString appResourcesPath()
{
    return QCoreApplication::applicationDirPath();
}


QString userResourcesPath()
{
    static QString path;
    if (path.isNull())
        path = QDir::homePath() + "/AppData/Local/MoonPlayer";
    return path;
}

//get ffmpeg's file path
QString ffmpegFilePath()
{
    static QString filename;
    if (filename.isNull())
        filename = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
    return filename;
}
