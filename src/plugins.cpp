#include "plugins.h"
#include <QDir>
#include <QHash>
#include "pyapi.h"
#include "settings_network.h"

/************************
 ** Initialize plugins **
 ************************/
int n_plugins = 0;
Plugin **plugins = NULL;

void initPlugins()
{
    PyRun_SimpleString("import sys");
#ifdef Q_OS_WIN
    PyRun_SimpleString("sys.path.append('plugins')");
#else
    PyRun_SimpleString("import os");
    PyRun_SimpleString("sys.path.insert(0, '/usr/share/moonplayer/plugins')");
    PyRun_SimpleString("sys.path.append(os.environ['HOME'] + '/.moonplayer/plugins')");
#endif

    //load plugins
    static Plugin *array[128];
    plugins = array;
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
        if (filename.startsWith("plugin_") && filename.endsWith(".py"))
        {
            array[n_plugins] = new Plugin(filename.section('.', 0, 0));
            n_plugins++;
        }
    }
}

/**************************
 *** Hosts & name table ***
 **************************/
static QHash<QString, Plugin*> host2plugin;
static QHash<QString, Plugin*> name2plugin;

Plugin *getPluginByHost(const QString &host)
{
    return host2plugin[host];
}

Plugin *getPluginByName(const QString &name)
{
    return name2plugin[name];
}


/*********************
 *** Plugin object ***
 ********************/
Plugin::Plugin(const QString &moduleName)
{
    //load module
    module = PyImport_ImportModule(moduleName.toUtf8().constData());
    if (module == NULL)
    {
        PyErr_Print();
        exit(-1);
    }
    name = moduleName.mid(7);
    //get search() and parse()
    searchFunc = PyObject_GetAttrString(module, "search");
    parseFunc = PyObject_GetAttrString(module, "parse");
    if (searchFunc == NULL || parseFunc == NULL)
    {
        PyErr_Print();
        exit(-1);
    }

    searchAlbumFunc = PyObject_GetAttrString(module, "search_album");
    if (searchAlbumFunc == NULL)
        PyErr_Clear();
    parseMarkFunc = PyObject_GetAttrString(module, "parse_mark");
    if (parseMarkFunc == NULL)
        PyErr_Clear();

    //get resources library
    libraryFunc = PyObject_GetAttrString(module, "library");
    if (libraryFunc == NULL)
        PyErr_Clear();

    PyObject *types = PyObject_GetAttrString(module, "movie_types");
    if (types)
    {
        for (int i = 0; i < PyList_Size(types); i++)
        {
            const char *str = PyString_AsString(PyList_GetItem(types, i));
            mvTypes << QString::fromUtf8(str);
        }
        Py_DecRef(types);
    }
    else
        PyErr_Clear();

    types = PyObject_GetAttrString(module, "tv_types");
    if (types)
    {
        for (int i = 0; i < PyList_Size(types); i++)
        {
            const char *str = PyString_AsString(PyList_GetItem(types, i));
            tvTypes << QString::fromUtf8(str);
        }
        Py_DecRef(types);
    }
    else
        PyErr_Clear();

    //get hosts
    PyObject *hosts = PyObject_GetAttrString(module, "hosts");
    if (hosts)
    {
        int size = PyTuple_Size(hosts);
        if (size < 0)
        {
            PyErr_Print();
            exit(EXIT_FAILURE);
        }
        for (int i = 0; i < size; i++)
        {
            const char *str = PyString_AsString(PyTuple_GetItem(hosts, i));
            host2plugin[QString::fromUtf8(str)] = this;
        }
    }
    else
        PyErr_Clear();

    // add to name table
    name2plugin[name] = this;
}

void Plugin::search(const QString &kw, int page)
{
    call_py_func_vsi(searchFunc, kw.toUtf8().constData(), page);
}

void Plugin::searchAlbum(const QString &kw, int page)
{
    Q_ASSERT(searchAlbumFunc);
    call_py_func_vsi(searchAlbumFunc, kw.toUtf8().constData(), page);
}

void Plugin::parse(const char *url, bool is_down)
{
    int options = (Settings::quality == Settings::SUPER) ? OPT_QL_SUPER : (Settings::quality == Settings::HIGH) ? OPT_QL_HIGH : 0;
    if (is_down)
        options |= OPT_DOWNLOAD;
    call_py_func_vsi(parseFunc, url, options);
}

void Plugin::parse_mark(const char *mark)
{
    PyObject *ret = PyObject_CallFunction(parseMarkFunc, "s", mark);
    if (ret == NULL)
        PyErr_Print();
    else
        Py_DecRef(ret);
}

void Plugin::library(bool is_movie, const QString &type, int page)
{
    Q_ASSERT(libraryFunc);
    PyObject *retVal = PyObject_CallFunction(libraryFunc, "isi", is_movie ? LIBTYPE_MOVIE : LIBTYPE_TV,
                                             type.toUtf8().constData(),
                                             page);
    if (retVal)
        Py_DecRef(retVal);
    else
        PyErr_Print();
}
