#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QStringList>
#include <Python.h>

//Convert python's string(include unicode) to QString
QString PyString_AsQString(PyObject *pystr);
QStringList PyList_AsQStringList(PyObject *tuple);

#endif // MP_UTILS_H
