#include "paths.h"
#include <QDir>
#include <QCoreApplication>
#include <QLibraryInfo>
#include <QStandardPaths>

QString getAppPath()
{
    static QString path;
    if (path.isNull())
    {
        QString appPath = QCoreApplication::applicationDirPath();
        if (appPath.endsWith("/bin"))
            path = appPath.replace("/bin", "/share/moonplayer");
        else
            path = "/usr/share/moonplayer";
    }
    return path;
}


QString getUserPath()
{
    static QString path;
    if (path.isNull())
    {
        const char *dataPath = getenv("XDG_DATA_HOME");
        path = dataPath ? QString::fromUtf8(dataPath) + "/moonplayer" : QDir::homePath() + "/.local/share/moonplayer";
    }
    return path;
}

// translation files
QString getQtTranslationsPath()
{
    return QLibraryInfo::location(QLibraryInfo::TranslationsPath);
}


//get ffmpeg's file path
QString ffmpegFilePath()
{
    return "ffmpeg";
}
