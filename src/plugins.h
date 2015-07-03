#ifndef PLUGINS_H
#define PLUGINS_H

#include "parser.h"

class Plugin : public Parser
{
public:
    Plugin(const QString &moduleName);
    void search(const QString &kw, int page);
    void searchAlbum(const QString &kw, int page);
    inline bool supportAlbum(){return searchAlbumFunc != 0;}
private:
    PyObject *searchFunc;
    PyObject *searchAlbumFunc;
};
extern Plugin **plugins;
extern int n_plugins;
void initPlugins(void);
Plugin *getPluginByHost(const QString &host);
Plugin *getPluginByName(const QString &name);

#endif // PLUGINS_H
