#include "plugin.h"
#include <QDir>
#include <QHash>
#include "platforms.h"
#include "pyapi.h"
#include "settings_plugins.h"

/************************
 ** Initialize plugins **
 ************************/
int n_plugins = 0;
Plugin **plugins = NULL;
Plugin *flvcd_parser = NULL;

void initPlugins()
{
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("reload(sys)");
    PyRun_SimpleString("sys.setdefaultencoding('utf8')");
    PyRun_SimpleString(QString("sys.path.insert(0, '%1/plugins')").arg(getAppPath()).toUtf8().constData());
    PyRun_SimpleString(QString("sys.path.append('%1/plugins')").arg(getUserPath()).toUtf8().constData());
#ifdef Q_OS_WIN
    if (win_debug)
    {
        PyRun_SimpleString("sys.stdout = open('CON ', 'w')");
        PyRun_SimpleString("sys.stderr = open('CON', 'w')");
        PyRun_SimpleString("sys.stdin  = open('CON', 'r')");
    }
#endif

    //load plugins
    static Plugin *array[128];
    plugins = array;
    QDir pluginsDir(getAppPath() + "/plugins");
    QStringList list = pluginsDir.entryList(QDir::Files, QDir::Name);
    pluginsDir = QDir(getUserPath() + "/plugins");
    list += pluginsDir.entryList(QDir::Files, QDir::Name);

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
