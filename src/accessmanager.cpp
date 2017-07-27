#include "accessmanager.h"
#include <QFile>
#include <QProcess>

QNetworkAccessManager *access_manager = 0;
QHash<QString,QByteArray> referer_table;
QHash<QString,QByteArray> ua_table;
QStringList unseekable_hosts;

QByteArray generateUA(const QUrl &url)
{
    static QByteArray default_ua;

    QString host = url.host();
    if (ua_table.contains(host))
        return ua_table[host];

    if (default_ua.isNull())
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
            default_ua = "moonplayer";
            return default_ua;
        }
        process.waitForFinished();
        default_ua = process.readAll().simplified();
    }
    return default_ua;
}
