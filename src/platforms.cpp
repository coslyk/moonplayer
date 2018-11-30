#include "platforms.h"
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>

QString getAppPath()
{
    static QString path;
    if (path.isNull())
    {
#if defined(Q_OS_LINUX)
        QString appPath = QCoreApplication::applicationDirPath();
        if (appPath.endsWith("/bin"))
            path = appPath.replace("/bin", "/share/moonplayer");
        else
            path = "/usr/share/moonplayer";
#elif defined(Q_OS_MAC)
        path = QCoreApplication::applicationDirPath().replace("/MacOS", "/Resources");
#elif defined(Q_OS_WIN)
        path = QCoreApplication::applicationDirPath();
#else
#error ERROR: Unsupported system!
#endif
    }
    return path;
}


QString getUserPath()
{
    static QString path;
    if (path.isNull())
    {
#if defined(Q_OS_LINUX)
        const char *dataPath = getenv("XDG_DATA_HOME");
        path = dataPath ? QString::fromUtf8(dataPath) + "/moonplayer" : QDir::homePath() + "/.local/share/moonplayer";
#elif defined(Q_OS_MAC)
        path = QDir::homePath() + "/Library/Application Support/MoonPlayer";
#elif defined(Q_OS_WIN)
        path = QDir::homePath() + "/AppData/Local/MoonPlayer";
#else
#error ERROR: Unsupported system!
#endif
    }
    return path;
}


void createUserPath()
{
    QDir dir;
    dir.mkpath(getUserPath() + "/plugins");
}


//videos' and pictures' path
QString getVideosPath()
{
    QString path;
    if (path.isNull())
        path = QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first();
    return path;
}

QString getPicturesPath()
{
    QString path;
    if (path.isNull())
        path = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
    return path;
}


//get ffmpeg's file path
QString ffmpegFilePath()
{
    static QString filename;
    if (filename.isNull())
    {
#if defined(Q_OS_LINUX)
        filename = "ffmpeg";
#elif defined(Q_OS_MAC)
        filename = QCoreApplication::applicationDirPath() + "/ffmpeg";
#else
#error ERROR: Unsupport system!
#endif
    }
    return filename;
}

// get parsers' upgrader path
QString parserUpgraderPath()
{
    static QString filename;
    if (filename.isNull())
    {
        filename = getAppPath() + "/upgrade-parsers.sh";
#if defined(Q_OS_MAC)
        if ((QFile::permissions(filename) & QFile::ExeOther) == 0) // make it excutable
            system(("chmod +x '" + filename + '\'').toUtf8().constData());
#endif
    }
    return filename;
}
