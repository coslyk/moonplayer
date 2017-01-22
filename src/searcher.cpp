#include "searcher.h"
#include "utils.h"
#include "pyapi.h"
#include <QDir>
#include "settings_player.h"

/************************
 ** Initialize plugins **
 ************************/
int n_searchers = 0;
Searcher **searchers = NULL;

void initSearchers()
{
    //load plugins
    static Searcher *array[128];
    searchers = array;

    QDir pluginsDir(Settings::path + "/plugins");
    QStringList list = pluginsDir.entryList(QDir::Files, QDir::Name);
#if defined(Q_OS_LINUX) || defined(Q_OS_MAC)
    pluginsDir = QDir(Settings::userPath + "/plugins");
    list += pluginsDir.entryList(QDir::Files, QDir::Name);
#endif

    while (!list.isEmpty())
    {
        QString filename = list.takeFirst();
        if (filename.startsWith("searcher_") && filename.endsWith(".py"))
        {
            array[n_searchers] = new Searcher(filename.section('.', 0, 0));
            n_searchers++;
        }
    }
}

/*********************
 ** Searcher object **
 ********************/
Searcher::Searcher(const QString &moduleName)
{
    //load module
    module = PyImport_ImportModule(moduleName.toUtf8().constData());
    if (module == NULL)
    {
        show_pyerr();
        exit(-1);
    }

    // Get name
    PyObject *_name = PyObject_GetAttrString(module, "searcher_name");
    if (_name == NULL)
    {
        show_pyerr();
        exit(EXIT_FAILURE);
    }
    name = PyString_AsQString(_name);
    Py_DecRef(_name);
    _name = NULL;

    //get search() function
    searchFunc = PyObject_GetAttrString(module, "search");
    if (searchFunc == NULL)
    {
        show_pyerr();
        exit(-1);
    }
}

void Searcher::search(const QString &kw, int page)
{
    PyObject *result = PyObject_CallFunction(searchFunc, "si", kw.toUtf8().constData(), page);
    if (result)
        Py_DecRef(result);
    else
        show_pyerr();
}
