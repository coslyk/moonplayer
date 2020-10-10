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
        filename = QCoreApplication::applicationDirPath() + QStringLiteral("/ffmpeg");
    return filename;
}

// hlsdl's file path
QString hlsdlFilePath()
{
    static QString filename;
    if (filename.isNull())
        filename = QCoreApplication::applicationDirPath() + QStringLiteral("/moonplayer-hlsdl");
    return filename;
}
