
#include "utils.h"

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
