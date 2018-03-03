#ifndef YKDLBRIDGE_H
#define YKDLBRIDGE_H


#include <QObject>
#include "parserbridge.h"
class QProcess;

class YkdlBridge : public ParserBridge
{
    Q_OBJECT
public:
    explicit YkdlBridge(QObject *parent = 0);

protected:
    void runParser(const QString &url);
    void parseOutput(const QByteArray &jsonData);
};

extern YkdlBridge ykdl_bridge;
#endif // YKDLBRIDGE_H
