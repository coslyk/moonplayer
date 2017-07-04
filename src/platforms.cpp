#include "platforms.h"
#include <QDir>
#include <QCoreApplication>

QString getAppPath()
{
    static QString path;
    if (path.isNull())
    {
#if defined(Q_OS_LINUX)
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
        path = QDir::homePath() + "/.moonplayer";
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
#else
#error ERROR: Unsupported system!
#endif
    if (!dir.exists("plugins"))
        dir.mkdir("plugins");
    if (!dir.exists("skins"))
        dir.mkdir("skins");
}



//get ffmpeg's file path
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

// get you-get's file path
QString yougetFilePath()
{
    static QString filename;
    if (filename.isNull())
    {
#if defined(Q_OS_WIN)
        filename = QCoreApplication::applicationDirPath() + "/you-get.exe";
#elif defined(Q_OS_LINUX)
        filename = QDir::homePath() + "/.moonplayer/you-get/you-get";
#elif defined(Q_OS_MAC)
        filename = QDir::homePath() + "/Library/Application Support/MoonPlayer/you-get/you-get";
#else
#error ERROR: Unsupport system!
#endif
    }
    return filename;
}


// get you-get's upgrader path
QString yougetUpgraderPath()
{
    static QString filename;
    if (filename.isNull())
    {
#if defined(Q_OS_LINUX)
        filename = "/usr/share/moonplayer/upgrade-you-get.sh";
#elif defined(Q_OS_MAC)
        filename = getAppPath().toUtf8() + "/upgrade-you-get.sh";
        if ((QFile::permissions(filename) & QFile::ExeOther) == 0) // make it excutable
            system(("chmod +x '" + filename + '\'').toUtf8().constData());
#else
#error ERROR: Unsupport system!
#endif
    }
    return filename;
}
