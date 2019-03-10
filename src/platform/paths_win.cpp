#include "paths.h"
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>

QString getAppPath()
{
    return QCoreApplication::applicationDirPath();
}


QString getUserPath()
{
    static QString path;
    if (path.isNull())
        path = QDir::homePath() + "/AppData/Local/MoonPlayer";
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
        filename = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
    return filename;
}
