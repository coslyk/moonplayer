#include "resplugin.h"
#include <QDir>
#include "platform/paths.h"

/************************
 ** Initialize plugins **
 ************************/
int n_resplugins = 0;
ResPlugin **resplugins = nullptr;

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
            bool ok = false;
            ResPlugin *plugin = new ResPlugin(filename.section('.', 0, 0), &ok);
            if (ok)
            {
                array[n_resplugins] = plugin;
                n_resplugins++;
            }
            else
            {
                delete plugin;
                qDebug("[plugin] Fails to load: %s", filename.toUtf8().constData());
            }
        }
    }
}

ResPlugin::ResPlugin(const QString &pluginName, bool *ok)
{
    //load module
    module = searchFunc = exploreFunc = loadItemFunc = nullptr;
    module = PyImport_ImportModule(pluginName.toUtf8().constData());
    if (module == nullptr)
    {
        printPythonException();
        *ok = false;
        return;
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
    if (searchFunc == nullptr || loadItemFunc == nullptr || exploreFunc == nullptr)
    {
        printPythonException();
        *ok = false;
        return;
    }

    //get tags
    PyObject *tags = PyObject_GetAttrString(module, "tags");
    if (tags == nullptr)
    {
        printPythonException();
        *ok = false;
        return;
    }
    tagsList = PyList_AsQStringList(tags);
    Py_DecRef(tags);

    //get countries
    PyObject *countries = PyObject_GetAttrString(module, "countries");
    if (countries == nullptr)
    {
        printPythonException();
        *ok = false;
        return;
    }
    countriesList = PyList_AsQStringList(countries);
    Py_DecRef(countries);

    // Add to __main__ namespace
    PyRun_SimpleString(QString("import %1").arg(pluginName).toUtf8().constData());
    *ok = true;
}

ResPlugin::~ResPlugin()
{
    Py_DecRef(module);
    Py_DecRef(searchFunc);
    Py_DecRef(exploreFunc);
    Py_DecRef(loadItemFunc);
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
        printPythonException();
}

void ResPlugin::search(const QString &key, int page)
{
    PyObject *retVal = PyObject_CallFunction(searchFunc, "si", key.toUtf8().constData(), page);
    if (retVal)
        Py_DecRef(retVal);
    else
        printPythonException();
}

void ResPlugin::loadItem(const QString &flag)
{
    PyObject *retVal = PyObject_CallFunction(loadItemFunc, "s", flag.toUtf8().constData());
    if (retVal)
        Py_DecRef(retVal);
    else
        printPythonException();
}
