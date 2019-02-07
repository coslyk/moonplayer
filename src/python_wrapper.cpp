#include "python_wrapper.h"

QString fetchPythonException()
{
    static PyObject *tracebackFunc = nullptr;
    PyObject *ptype, *pvalue, *ptraceback;
    QStringList tracebacks;

    // Get error data
    PyErr_Fetch(&ptype, &pvalue, &ptraceback);
    PyErr_NormalizeException(&ptype, &pvalue, &ptraceback);

    // See if we can get a full traceback
    if (tracebackFunc == nullptr)
    {
        PyObject *pyth_module = PyImport_ImportModule("traceback");
        if (pyth_module)
            tracebackFunc = PyObject_GetAttrString(pyth_module, "format_exception");
    }
    if (tracebackFunc && PyCallable_Check(tracebackFunc)) {
        PyObject *retVal = PyObject_CallFunctionObjArgs(tracebackFunc, ptype, pvalue, ptraceback, nullptr);
        tracebacks = PyList_AsQStringList(retVal);
        Py_DecRef(retVal);
    }

    Py_DecRef(ptype);
    Py_DecRef(pvalue);
    Py_DecRef(ptraceback);

    return tracebacks.join("");
}

void printPythonException()
{
    qDebug("%s", fetchPythonException().toUtf8().constData());
}


/* Convert a Python string (including Unicode in Py2 and Bytes in Py3) to a QString */
QString PyString_AsQString(PyObject *pystr)
{
    // Unicode
    if (PyUnicode_Check(pystr))
    {
#if PY_MAJOR_VERSION >= 3
        QString s = QString::fromUtf8(PyUnicode_AsUTF8(pystr));
#else
        PyObject *utf8str = PyUnicode_AsUTF8String(pystr);
        QString s = QString::fromUtf8(PyString_AsString(utf8str));
        Py_DecRef(utf8str);
#endif
        return s;
    }

    // Bytes
#if PY_MAJOR_VERSION >= 3
    else if (PyByteArray_Check(pystr))
        return QString::fromUtf8(PyByteArray_AsString(pystr));
#else
    else if (PyString_Check(pystr))
        return QString::fromUtf8(PyString_AsString(pystr));
#endif
    else
        return QString();
}

/* Convert a Python object to a QVariant */
QVariant PyObject_AsQVariant(PyObject *obj)
{
    // Dict -> QVariantHash
    if (PyDict_Check(obj))
    {
        QVariantHash result;
        Py_ssize_t pos = 0;
        PyObject *pykey, *pyval;
        while (PyDict_Next(obj, &pos, &pykey, &pyval))
        {
            QString key = PyString_AsQString(pykey);
            result[key] = PyObject_AsQVariant(pyval);
        }
        return result;
    }
    // List -> QVariantList
    else if (PyList_Check(obj))
    {
        QVariantList result;
        Py_ssize_t len = PyList_Size(obj);
        for (Py_ssize_t i = 0; i < len; i++)
        {
            PyObject *item = PyList_GetItem(obj, i);
            result << PyObject_AsQVariant(item);
        }
        return result;
    }
    // Tuple -> QVariantList
    else if (PyTuple_Check(obj))
    {
        QVariantList result;
        Py_ssize_t len = PyTuple_Size(obj);
        for (Py_ssize_t i = 0; i < len; i++)
        {
            PyObject *item = PyTuple_GetItem(obj, i);
            result << PyObject_AsQVariant(item);
        }
        return result;
    }
    // Long
    else if (PyLong_Check(obj))
        return PyLong_AsLongLong(obj);

    // Int
#if PY_MAJOR_VERSION == 2
    else if (PyInt_Check(obj))
        return (long long) PyInt_AsLong(obj);
#endif

    // Float
    else if (PyFloat_Check(obj))
        return PyFloat_AsDouble(obj);

    // String
#if PY_MAJOR_VERSION >= 3
    else if (PyUnicode_Check(obj) || PyByteArray_Check(obj))
#else
    else if (PyUnicode_Check(obj) || PyString_Check(obj))
#endif
        return PyString_AsQString(obj);

    // Unknown
    else
    {
        qDebug("Error: convert an PyObject of unknown type to QVariant.");
        return QVariant();
    }
}

/* Convert a Python list to a QStringList */
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
