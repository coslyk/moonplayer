#include "accessmanager.h"
#include <QFile>
#include <QProcess>

QNetworkAccessManager *access_manager = 0;
QHash<QString,QByteArray> referer_table;
QStringList unseekable_hosts;

QByteArray defaultUA()
{
    static QByteArray ua;
    if (ua.isNull())
    {
        QStringList args;
        args << "-c" << "import sys; print('Python-urllib/%d.%d' % sys.version_info[:2])";
        QProcess process;
        if (QFile::exists("/usr/bin/python3"))
            process.start("/usr/bin/python3", args, QProcess::ReadOnly);
        else if (QFile::exists("/usr/local/bin/python3"))
            process.start("/usr/local/bin/python3", args, QProcess::ReadOnly);
        else
        {
            ua = "moonplayer";
            return ua;
        }
        process.waitForFinished();
        ua = process.readAll().simplified();
    }
    return ua;
}
