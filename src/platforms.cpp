#include "platforms.h"
#include <QDir>
#include <QCoreApplication>

QString getAppPath()
{
#if defined(Q_OS_LINUX)
    return "/usr/share/moonplayer";
#elif defined(Q_OS_MAC)
    return QCoreApplication::applicationDirPath().replace("/MacOS", "/Resources");
#elif defined(Q_OS_WIN)
    return QCoreApplication::applicationDirPath();
#else
#error ERROR: Unsupported system!
#endif
}


QString createUserPath()
{
    QDir dir = QDir::home();
#if defined(Q_OS_LINUX)
    if (!dir.cd(".moonplayer"))
    {
        dir.mkdir(".moonplayer");
        dir.cd(".moonplayer");
    }
#elif defined(Q_OS_MAC)
    dir.cd("Library");
    dir.cd("Application Support");
    if (!dir.cd("MoonPlayer"))
    {
        dir.mkdir("MoonPlayer");
        dir.cd("MoonPlayer");
    }
#elif defined(Q_OS_WIN)
    dir.cd("AppData");
    dir.cd("Local");
    if (!dir.cd("MoonPlayer"))
    {
        dir.mkdir("MoonPlayer");
        dir.cd("MoonPlayer");
    }
#endif
    if (!dir.exists("plugins"))
        dir.mkdir("plugins");
    if (!dir.exists("skins"))
        dir.mkdir("skins");

#if defined(Q_OS_LINUX)
    return QDir::homePath() + "/.moonplayer";
#elif defined(Q_OS_MAC)
    return QDir::homePath() + "/Library/Application Support/MoonPlayer";
#elif defined(Q_OS_WIN)
    return QDir::homePath() + "/AppData/Local/MoonPlayer";
#else
#error ERROR: Unsupported system!
#endif
}



//get ffmpeg's file name
QString ffmpegFilePath()
{
    static QString filename;
    if (filename.isNull())
    {
#if defined(Q_OS_WIN)
        filename = QCoreApplication::applicationDirPath() + "/ffmpeg.exe";
#elif defined(Q_OS_LINUX)
        filename = "ffmpeg";
#elif defined(Q_OS_MAC)
        filename = QCoreApplication::applicationDirPath() + "/ffmpeg";
#else
#error ERROR: Unsupport system!
#endif
    }
    return filename;
}
