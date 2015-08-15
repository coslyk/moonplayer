#ifndef SEARCHER_H
#define SEARCHER_H


#include <Python.h>
#include <QString>

class Searcher
{
public:
    Searcher(const QString &moduleName);
    inline QString &getName(){return name;}
    void search(const QString &kw, int page);
private:
    QString name;
    PyObject *module;
    PyObject *searchFunc;
};

extern Searcher **searchers;
extern int n_searchers;
void initSearchers(void);

#endif // SEARCHER_H
