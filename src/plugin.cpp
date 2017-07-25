#include "plugin.h"
#include <QDir>
#include <QHash>
#include "platforms.h"
#include "pyapi.h"
#include "settings_plugins.h"

/************************
 ** Initialize plugins **
 ************************/

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
        PyRun_SimpleString("sys.stdout = open('CON', 'w')");
        PyRun_SimpleString("sys.stderr = open('CON', 'w')");
        PyRun_SimpleString("sys.stdin  = open('CON', 'r')");
    }
#endif
}
