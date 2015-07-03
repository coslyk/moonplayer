#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <Python.h>

class Parser
{
public:
    Parser(const QString &moduleName);
    void parse(const char *url, bool is_down);
    inline QString &getName(){return name;}
protected:
    PyObject *module;
    PyObject *parseFunc;
    QString name;
};

#endif // PARSER_H
