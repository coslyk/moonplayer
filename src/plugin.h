#ifndef PLUGINS_H
#define PLUGINS_H

#include <Python.h>
#include <QString>

class Plugin
{
public:
    Plugin(const QString &moduleName);
    void parse(const char *url, bool is_down);
private:
    PyObject *module;
    PyObject *parseFunc;
};
extern Plugin **plugins;
extern int n_plugins;
extern Plugin *flvcd_parser;
extern QString plugins_msg;
void initPlugins(void);
Plugin *getPluginByHost(const QString &host);

#endif // PLUGINS_H
