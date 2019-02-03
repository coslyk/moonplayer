#include "paths.h"
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>


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
