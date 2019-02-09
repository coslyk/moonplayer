#include "paths.h"
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>

QString getAppPath()
{
    static QString path;
    if (path.isNull())
        path = QCoreApplication::applicationDirPath().replace("/MacOS", "/Resources");
    return path;
}


QString getUserPath()
{
    static QString path;
    if (path.isNull())
        path = QDir::homePath() + "/Library/Application Support/MoonPlayer";
    return path;
}

QString getQtTranslationsPath()
{
    return getAppPath() + "/qt/translations";
}

//get ffmpeg's file path
QString ffmpegFilePath()
{
    static QString filename;
    if (filename.isNull())
        filename = QCoreApplication::applicationDirPath() + "/ffmpeg";
    return filename;
}
