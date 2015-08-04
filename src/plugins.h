#ifndef PLUGINS_H
#define PLUGINS_H

#include <Python.h>
#include <QString>

class Plugin
{
public:
    Plugin(const QString &moduleName);
    void parse(const char *url, bool is_down);
    inline QString &getName(){return name;}
    void search(const QString &kw, int page);
    void searchAlbum(const QString &kw, int page);
    inline bool supportAlbum(){return searchAlbumFunc != 0;}
private:
    PyObject *module;
    PyObject *parseFunc;
    QString name;
    PyObject *searchFunc;
    PyObject *searchAlbumFunc;
};
extern Plugin **plugins;
extern int n_plugins;
extern QString plugins_msg;
void initPlugins(void);
Plugin *getPluginByHost(const QString &host);
Plugin *getPluginByName(const QString &name);

#endif // PLUGINS_H
