#ifndef PARSERYOUTUBEDL_H
#define PARSERYOUTUBEDL_H

#include "parserbase.h"
class QProcess;
class QWidget;

class ParserYoutubeDL : public ParserBase
{
    Q_OBJECT
public:
    explicit ParserYoutubeDL(QObject *parent = 0);
    ~ParserYoutubeDL();

protected:
    void runParser(const QString &url);

private:
    QProcess *process;
    QWidget *msgWindow;

private slots:
    void parseOutput(void);
};

extern ParserYoutubeDL *parser_youtubedl;

#endif // PARSERYOUTUBEDL_H
