#include "resplugin.h"
#include <QDir>
#include "utils.h"
#include "pyapi.h"
#ifdef Q_OS_WIN
#include "settings_player.h"
#endif

/************************
 ** Initialize plugins **
 ************************/
int n_resplugins = 0;
ResPlugin **resplugins = NULL;

void initResPlugins()
{
    static ResPlugin *array[128];
    resplugins = array;
#ifdef Q_OS_WIN
    QDir pluginsDir = QDir(Settings::path);
    pluginsDir.cd("plugins");
    QStringList list = pluginsDir.entryList(QDir::Files, QDir::Name);
#else
    QDir pluginsDir = QDir("/usr/share/moonplayer/plugins");
    QStringList list = pluginsDir.entryList(QDir::Files, QDir::Name);
    pluginsDir = QDir::home();
    pluginsDir.cd(".moonplayer");
    pluginsDir.cd("plugins");
    list += pluginsDir.entryList(QDir::Files, QDir::Name);
#endif
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
    loadItemFunc = PyObject_GetAttrString(module, "load_item");
    if (searchFunc == NULL || loadItemFunc == NULL)
    {
        show_pyerr();
        exit(-1);
    }

    //get tags
    PyObject *tags = PyObject_GetAttrString(module, "tags");
    if (tags == NULL)
    {
        show_pyerr();
        exit(-1);
    }
    tagsList = PyList_AsQStringList(tags);
    Py_DecRef(tags);

    //get countries
    PyObject *countries = PyObject_GetAttrString(module, "countries");
    if (countries == NULL)
    {
        show_pyerr();
        exit(-1);
    }
    countriesList = PyList_AsQStringList(countries);
    Py_DecRef(countries);
}

void ResPlugin::search(const QString &tag, const QString &country, int page)
{
    PyObject *dict = PyDict_New();
    PyDict_SetItemString(dict, "tag", PyString_FromString(tag.toUtf8().constData()));
    PyDict_SetItemString(dict, "country", PyString_FromString(country.toUtf8().constData()));
    PyDict_SetItemString(dict, "page", PyInt_FromLong(page));
    PyObject *retVal = PyObject_CallFunction(searchFunc, "O", dict);
    if (retVal)
        Py_DecRef(retVal);
    else
        show_pyerr();
}

void ResPlugin::searchByKey(const QString &key, int page)
{
    PyObject *dict = PyDict_New();
    PyDict_SetItemString(dict, "key", PyString_FromString(key.toUtf8().constData()));
    PyDict_SetItemString(dict, "page", PyInt_FromLong(page));
    PyObject *retVal = PyObject_CallFunction(searchFunc, "O", dict);
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
