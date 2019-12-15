#ifndef PARSERYOUTUBEDL_H
#define PARSERYOUTUBEDL_H

#include "parserBase.h"
class QProcess;

class ParserYoutubeDL : public ParserBase
{
    Q_OBJECT
public:
    inline static ParserYoutubeDL* instance() { return &s_instance; }
    explicit ParserYoutubeDL(QObject *parent = 0);
    ~ParserYoutubeDL();

protected:
    void runParser(const QUrl &url);

private slots:
    void parseOutput(void);

private:
    QProcess *process;
    
    static ParserYoutubeDL s_instance;
};

#endif // PARSERYOUTUBEDL_H
