#include "plugins.h"
#include <QDir>
#include <QHash>
#include "pyapi.h"
#include "settings_plugins.h"
#ifdef Q_OS_WIN
#include "settings_player.h"
#endif

/************************
 ** Initialize plugins **
 ************************/
int n_plugins = 0;
Plugin **plugins = NULL;
Plugin *flvcd_parser = NULL;
QString plugins_msg;

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
    foreach (QString item, list) {
        if ((item.startsWith("plugin_") || item.startsWith("res_")) && item.endsWith(".py"))
            plugins_msg += item + "\n";
    }
#else
    plugins_msg = "System plugins:\n    ";
    QDir pluginsDir = QDir("/usr/share/moonplayer/plugins");
    QStringList list = pluginsDir.entryList(QDir::Files, QDir::Name);
    foreach (QString item, list) {
        if ((item.startsWith("plugin_") || item.startsWith("res_") || item.startsWith("searcher_")) &&
                item.endsWith(".py"))
            plugins_msg += item + "\n    ";
    }
    plugins_msg += "\nPlugins installed by user:\n    ";
    pluginsDir = QDir::home();
    pluginsDir.cd(".moonplayer");
    pluginsDir.cd("plugins");
    QStringList list_users = pluginsDir.entryList(QDir::Files, QDir::Name);
    list += list_users;
    foreach (QString item, list_users) {
        if ((item.startsWith("plugin_") || item.startsWith("res_") || item.startsWith("searcher_")) &&
                item.endsWith(".py"))
            plugins_msg += item + "\n    ";
    }
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
    flvcd_parser = new Plugin("flvcd_parser");
}

/**************************
 *** Hosts & name table ***
 **************************/
static QHash<QString, Plugin*> host2plugin;

Plugin *getPluginByHost(const QString &host)
{
    return host2plugin[host];
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
        show_pyerr();
        exit(-1);
    }

    // Get parse() function
    parseFunc = PyObject_GetAttrString(module, "parse");
    if (parseFunc == NULL)
    {
        show_pyerr();
        exit(-1);
    }

    //get hosts
    PyObject *hosts = PyObject_GetAttrString(module, "hosts");
    if (hosts)
    {
        int size = PyTuple_Size(hosts);
        if (size < 0)
        {
            show_pyerr();
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
}

void Plugin::parse(const char *url, bool is_down)
{
    int options = 0;
	switch (Settings::quality)
	{
	case Settings::_1080P:options |= OPT_QL_1080P;
	case Settings::SUPER: options |= OPT_QL_SUPER;
	case Settings::HIGH:  options |= OPT_QL_HIGH;
	default: break;
	}
    if (is_down)
        options |= OPT_DOWNLOAD;
    call_py_func_vsi(parseFunc, url, options);
}
