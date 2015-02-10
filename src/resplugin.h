#ifndef RESPLUGIN_H
#define RESPLUGIN_H

#include <Python.h>
#include <QStringList>

class ResPlugin
{
public:
    ResPlugin(const QString &pluginName);
    void search(const QString &tag, const QString &country, int page);
    void searchByKey(const QString &key, int page);
    void loadItem(const QByteArray &flag);
    inline QString &getName(){return name;}

    QStringList tagsList;
    QStringList countriesList;

private:
    QString name;
    PyObject *module;
    PyObject *searchFunc;
    PyObject *loadItemFunc;
};
extern ResPlugin **resplugins;
extern int n_resplugins;
void initResPlugins(void);

#endif // RESPLUGIN_H
