#include "plugins.h"
#include <QDir>
#include <QHash>
#include "pyapi.h"
#include "settings_network.h"
#ifdef Q_OS_WIN
#include "settings_player.h"
#endif

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
Plugin::Plugin(const QString &moduleName) : Parser(moduleName)
{
    //get search() function
    searchFunc = PyObject_GetAttrString(module, "search");
    if (searchFunc == NULL)
    {
        PyErr_Print();
        exit(-1);
    }

    searchAlbumFunc = PyObject_GetAttrString(module, "search_album");
    if (searchAlbumFunc == NULL)
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
