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
    ~YouGetBridge();

protected:
    void runParser(const QString &url);

private:
    QProcess *process;

private slots:
    void parseOutput(void);
};

extern YouGetBridge you_get_bridge;

#endif // YOUGETBRIDGE_H
