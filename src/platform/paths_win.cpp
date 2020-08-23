#include "paths.h"
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>


QString userResourcesPath()
{
    static QString path;
    if (path.isNull())
        path = QDir::homePath() + QStringLiteral("/AppData/Local/MoonPlayer");
    return path;
}

// FFmpeg's file path
QString ffmpegFilePath()
{
    static QString filename;
    if (filename.isNull())
        filename = QCoreApplication::applicationDirPath() + QStringLiteral("/ffmpeg.exe");
    return filename;
}

// hlsdl's file path
QString hlsdlFilePath()
{
    static QString filename;
    if (filename.isNull())
        filename = QCoreApplication::applicationDirPath() + QStringLiteral("/hlsdl.exe");
    return filename;
}
