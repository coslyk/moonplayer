#include "paths.h"
#include <QDir>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QStandardPaths>

QString appResourcesPath()
{
    static QString path;
    if (path.isNull())
    {
        QString appPath = QCoreApplication::applicationDirPath();
        if (appPath.endsWith(QStringLiteral("/bin")))
            path = appPath.replace(QStringLiteral("/bin"), QStringLiteral("/share/moonplayer"));
        else
            path = QStringLiteral("/usr/local/share/moonplayer");
    }
    return path;
}


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


//get ffmpeg's file path
QString ffmpegFilePath()
{
    return QStringLiteral("ffmpeg");
}
