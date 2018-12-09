#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QStringList>
#include <QUrl>
#include <Python.h>

//Convert python's string(include unicode) to QString
QString PyString_AsQString(PyObject *pystr);
QStringList PyList_AsQStringList(PyObject *listobj);

QString secToTime(int second, bool use_format = false);

//Read .xspf playlists
void readXspf(const QByteArray& xmlpage, QStringList& result);

//Save cookies to disk
bool saveCookies(const QUrl &url, const QString &filename);

#endif // MP_UTILS_H
