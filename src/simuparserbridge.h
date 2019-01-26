#ifndef SIMUPARSERBRIDGE_H
#define SIMUPARSERBRIDGE_H

#include "parserbridge.h"
#include <Python.h>
class SimuParser;

class SimuParserBridge : public ParserBridge
{
    Q_OBJECT
public:
    SimuParserBridge(QObject *parent = 0);
    void initParser(void);

protected:
    void runParser(const QString &url);

private:
    SimuParser *parser;

private slots:
    void onParseFinished(PyObject *dict);
};

extern SimuParserBridge simuParserBridge;

#endif // SIMUPARSERBRIDGE_H
