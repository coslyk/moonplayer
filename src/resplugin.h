#ifndef RESPLUGIN_H
#define RESPLUGIN_H

#include "python_wrapper.h"
#include <QStringList>

class ResPlugin
{
public:
    ResPlugin(const QString &pluginName);
    void explore(const QString &tag, const QString &country, int page);
    void search(const QString &key, int page);
    void loadItem(const QString &flag);
    inline QString &getName(){return name;}

    QStringList tagsList;
    QStringList countriesList;

private:
    QString name;
    PyObject *module;
    PyObject *searchFunc;
    PyObject *exploreFunc;
    PyObject *loadItemFunc;
};
extern ResPlugin **resplugins;
extern int n_resplugins;
void initResPlugins(void);

#endif // RESPLUGIN_H
