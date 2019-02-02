#ifndef YKDLBRIDGE_H
#define YKDLBRIDGE_H


#include <QObject>
#include "parserbridge.h"
class QWidget;
class QProcess;

class YkdlBridge : public ParserBridge
{
    Q_OBJECT
public:
    explicit YkdlBridge(QObject *parent = 0);
    ~YkdlBridge();
    static bool isSupported(const QString &host);

protected:
    void runParser(const QString &url);

private:
    QProcess *process;
    QWidget *msgWindow;

private slots:
    void parseOutput(void);
};

extern YkdlBridge *ykdl_bridge;
#endif // YKDLBRIDGE_H
