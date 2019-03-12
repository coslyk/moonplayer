#ifndef PYTHON_WRAPPER_H
#define PYTHON_WRAPPER_H

#include <QString>
#include <QStringList>
#include <QVariant>
#include <QCoreApplication>
#include <QDir>

// Python3 defines macro "slots", which is conflict with Qt
#pragma push_macro("slots")
#undef slots
#include <Python.h>
#pragma pop_macro("slots")

// Python executable file
#if defined(Q_OS_MAC)
#define PYTHON_BIN "python"
#elif defined(Q_OS_WIN)
#define PYTHON_BIN (QDir(QCoreApplication::applicationDirPath()).filePath("python3.exe"))
#elif PY_MAJOR_VERSION >= 3
#define PYTHON_BIN "python3"
#else
#define PYTHON_BIN "python2"
#endif

// Python3 compatibility
#if PY_MAJOR_VERSION >= 3
#define PyString_FromString(str) PyUnicode_FromString(str)
#endif

// Error handling
QString fetchPythonException();
void printPythonException();


// Convert python's string(include unicode) to QString
QString PyString_AsQString(PyObject *pystr);
QStringList PyList_AsQStringList(PyObject *listobj);

// Convert a Python object to a QVariant
QVariant PyObject_AsQVariant(PyObject *obj);

#endif // PYTHON_WRAPPER_H
