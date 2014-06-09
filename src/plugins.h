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
    void library(bool is_movie, const QString &type, int page);
    inline bool supportAlbum(){return searchAlbumFunc != 0;}
    inline bool supportLibrary(){return libraryFunc != 0;}
    inline QString &getName(){return name;}
    QStringList tvTypes;
    QStringList mvTypes;
private:
    PyObject *module;
    PyObject *searchFunc;
    PyObject *searchAlbumFunc;
    PyObject *parseFunc;
    PyObject *libraryFunc;
    QString name;
};
extern Plugin **plugins;
extern int n_plugins;
void initPlugins(void);
Plugin *getPluginByHost(const QString &host);

#endif // PLUGINS_H
