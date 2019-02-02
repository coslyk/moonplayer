#ifndef SIMUPARSERBRIDGE_H
#define SIMUPARSERBRIDGE_H

#include "parserbridge.h"
class SimuParser;

class SimuParserBridge : public ParserBridge
{
    Q_OBJECT
public:
    SimuParserBridge(QObject *parent = 0);
    void onParseFinished(const QVariantHash &data);

protected:
    void runParser(const QString &url);

private:
    SimuParser *parser;
};

extern SimuParserBridge *simuParserBridge;

#endif // SIMUPARSERBRIDGE_H
