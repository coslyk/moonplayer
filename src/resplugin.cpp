#include "resplugin.h"
#include <QDir>
#include "utils.h"
#include "platforms.h"
#include "pyapi.h"

/************************
 ** Initialize plugins **
 ************************/
int n_resplugins = 0;
ResPlugin **resplugins = NULL;

void initResPlugins()
{
    static ResPlugin *array[128];
    resplugins = array;

    QDir pluginsDir = QDir(getUserPath() + "/plugins");
    QStringList list = pluginsDir.entryList(QDir::Files, QDir::Name);

    while (!list.isEmpty())
    {
        QString filename = list.takeFirst();
        if (filename.startsWith("res_") && filename.endsWith(".py"))
        {
            array[n_resplugins] = new ResPlugin(filename.section('.', 0, 0));
            n_resplugins++;
        }
    }
}

ResPlugin::ResPlugin(const QString &pluginName)
{
    //load module
    module = PyImport_ImportModule(pluginName.toUtf8().constData());
    if (module == NULL)
    {
        show_pyerr();
        exit(-1);
    }

    //get name
    PyObject *_name = PyObject_GetAttrString(module, "res_name");
    if (_name)
    {
        name = PyString_AsQString(_name);
        Py_DecRef(_name);
    }
    else
    {
        PyErr_Clear();
        name = pluginName.mid(4);
    }

    //get search() and load_item()
    searchFunc = PyObject_GetAttrString(module, "search");
    exploreFunc = PyObject_GetAttrString(module, "explore");
    loadItemFunc = PyObject_GetAttrString(module, "load_item");
    if (searchFunc == NULL || loadItemFunc == NULL || exploreFunc == NULL)
    {
        show_pyerr();
        exit(EXIT_FAILURE);
    }

    //get tags
    PyObject *tags = PyObject_GetAttrString(module, "tags");
    if (tags == NULL)
    {
        show_pyerr();
        exit(EXIT_FAILURE);
    }
    tagsList = PyList_AsQStringList(tags);
    Py_DecRef(tags);

    //get countries
    PyObject *countries = PyObject_GetAttrString(module, "countries");
    if (countries == NULL)
    {
        show_pyerr();
        exit(EXIT_FAILURE);
    }
    countriesList = PyList_AsQStringList(countries);
    Py_DecRef(countries);

    // Add to __main__ namespace
    PyRun_SimpleString(QString("import %1").arg(pluginName).toUtf8().constData());
}

void ResPlugin::explore(const QString &tag, const QString &country, int page)
{
    PyObject *retVal = PyObject_CallFunction(exploreFunc, "ssi",
                                             tag.toUtf8().constData(),
                                             country.toUtf8().constData(),
                                             page);
    if (retVal)
        Py_DecRef(retVal);
    else
        show_pyerr();
}

void ResPlugin::search(const QString &key, int page)
{
    PyObject *retVal = PyObject_CallFunction(searchFunc, "si", key.toUtf8().constData(), page);
    if (retVal)
        Py_DecRef(retVal);
    else
        show_pyerr();
}

void ResPlugin::loadItem(const QByteArray &flag)
{
    PyObject *retVal = PyObject_CallFunction(loadItemFunc, "s", flag.constData());
    if (retVal)
        Py_DecRef(retVal);
    else
        show_pyerr();
}
