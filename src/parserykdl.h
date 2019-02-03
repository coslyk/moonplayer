#ifndef PARSERYKDL_H
#define PARSERYKDL_H


#include <QObject>
#include "parserbase.h"
class QWidget;
class QProcess;

class ParserYkdl : public ParserBase
{
    Q_OBJECT
public:
    explicit ParserYkdl(QObject *parent = 0);
    ~ParserYkdl();
    static bool isSupported(const QString &host);

protected:
    void runParser(const QString &url);

private:
    QProcess *process;
    QWidget *msgWindow;

private slots:
    void parseOutput(void);
};

extern ParserYkdl *parser_ykdl;
#endif // PARSERYKDL_H
