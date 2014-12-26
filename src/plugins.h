#ifndef PLUGINS_H
#define PLUGINS_H

#include <QObject>
#include <Python.h>
#include <QStringList>

class Plugin
{
public:
    Plugin(const QString &moduleName);
    ~Plugin();
    void search(const QString &kw, int page);
    void searchAlbum(const QString &kw, int page);
    void parse(const char *url, bool is_down);
    void parse_mark(const char *mark);
    inline bool supportAlbum(){return searchAlbumFunc != 0;}
    inline QString &getName(){return name;}
private:
    PyObject *module;
    PyObject *searchFunc;
    PyObject *searchAlbumFunc;
    PyObject *parseFunc;
    PyObject *parseMarkFunc;
    QString name;
};
extern Plugin **plugins;
extern int n_plugins;
void initPlugins(void);
Plugin *getPluginByHost(const QString &host);
Plugin *getPluginByName(const QString &name);

#endif // PLUGINS_H
