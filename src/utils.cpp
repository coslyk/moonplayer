#include "utils.h"
#include "accessmanager.h"
#include <QFile>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkCookie>
#include <QNetworkCookieJar>
#ifdef Q_OS_WIN
#include "settings_player.h"
#endif

QString PyString_AsQString(PyObject *pystr)
{
    if (PyUnicode_Check(pystr))
    {
        PyObject *utf8str = PyUnicode_AsUTF8String(pystr);
        QString s = QString::fromUtf8(PyString_AsString(utf8str));
        Py_DecRef(utf8str);
        return s;
    }
    else if (PyString_Check(pystr))
        return QString::fromUtf8(PyString_AsString(pystr));
    else
        return QString();
}

QStringList PyList_AsQStringList(PyObject *listobj)
{
    QStringList list;
    if (!PyList_Check(listobj))
        return list;
    int len = PyList_Size(listobj);
    for (int i = 0; i < len; i++)
    {
        PyObject *item = PyList_GetItem(listobj, i);
        QString s = PyString_AsQString(item);
        list << s;
    }
    return list;
}

QString secToTime(int second, bool use_format)
{
    static QString format = "<span style=\" font-size:14pt; font-weight:600;color:#00ff00;\">%1:%2:%3</span>";
    QString  hour = QString::number(second / 3600);
    QString min = QString::number((second % 3600) / 60);
    QString sec = QString::number(second % 60);
    if (min.length() == 1)
        min.prepend('0');
    if (sec.length() == 1)
        sec.prepend('0');
    if (use_format)
        return format.arg(hour, min, sec);
    else
        return QString("%1:%2:%3").arg(hour, min, sec);
}

void readXspf(const QByteArray &xmlpage, QStringList &result)
{
    QDomDocument doc;
    QDomElement elem;
    QString title, location;
    if (!doc.setContent(xmlpage))
        return;
    elem = doc.documentElement(); //<playlist>
    elem = elem.firstChildElement("trackList"); //<tracklist>
    elem = elem.firstChildElement("track"); //first <track>
    while (!elem.isNull())
    {
        title = elem.firstChildElement("title").text();
        location = elem.firstChildElement("location").text();
        result << title;
        result << location;
        elem = elem.nextSiblingElement("track"); //next <track>
    }
}

//get ffmpeg's file name
QString getFFmpegFile()
{
    static QString filename;
    if (filename.isNull())
    {
#if defined(Q_OS_WIN)
        QDir dir(Settings::path);
        if (dir.exists("ffmpeg.exe"))
            dir.filePath("ffmpeg.exe");
        else
            filename = "";

#elif defined(Q_OS_LINUX)
        QDir dir = QDir::home();
        dir.cd(".moonplayer");
        if (dir.exists("ffmpeg"))
            filename = dir.filePath("ffmpeg");
        else if (QDir("/usr/share/moonplayer").exists("ffmpeg"))
            filename = "/usr/share/moonplayer/ffmpeg";
        else
            filename = "";
#elif defined(Q_OS_MAC)
        filename = "";
#else
#error ERROR: Unsupport system!
#endif
    }
    return filename;
}

//save cookies to disk
bool saveCookies(const QUrl &url, const QString &filename)
{
    QList<QNetworkCookie> cookies = access_manager->cookieJar()->cookiesForUrl(url);
    QByteArray content;

    foreach (QNetworkCookie cookie, cookies) {
        // convert to mozilla's format
        QString row = cookie.toRawForm();
        QString name = row.section('=', 0, 0);
        QString value = row.section('=', 1).section(';', 0, 0);
        QString domain, path;
        if (row.contains("domain"))
            domain = row.section("domain=", 1).section(';', 0, 0);
        if (row.contains("path"))
            path = row.section("path=", 1).section(';', 0, 0);
        content += QString("%1\tTRUE\t%2\tFALSE\t\t%3\t%4\n").arg(domain, path, name, value).toUtf8();
    }
    if (content.isEmpty())  //no cookies needed
        return false;
    else
    {
        QFile file(filename);
        if (!file.open(QFile::WriteOnly | QFile::Text))
            return false;
        file.write("# Netscape HTTP Cookie File\n"
           "# http://curl.haxx.se/rfc/cookie_spec.html\n"
           "# This is a generated file!  Do not edit.\n\n");
        file.write(content);
        file.close();
        return true;
    }
}
