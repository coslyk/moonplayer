#include "utils.h"
#include <QDomDocument>
#include <QDomElement>

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
