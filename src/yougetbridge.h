#ifndef YOUGETBRIDGE_H
#define YOUGETBRIDGE_H

#include "parserbridge.h"
class QProcess;
class SelectionDialog;

class YouGetBridge : public ParserBridge
{
    Q_OBJECT
public:
    explicit YouGetBridge(QObject *parent = 0);

public slots:
    void updateYouGet(void);

protected:
    void runParser(const QString &url);
    void parseOutput(const QByteArray &jsonData);
};

extern YouGetBridge you_get_bridge;

#endif // YOUGETBRIDGE_H
