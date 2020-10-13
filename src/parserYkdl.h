#ifndef PARSERYKDL_H
#define PARSERYKDL_H


#include "parserBase.h"
#include <QProcess>

class ParserYkdl : public ParserBase
{
    Q_OBJECT
public:
    explicit ParserYkdl(QObject *parent = 0);
    ~ParserYkdl();
    inline static ParserYkdl* instance() { return &s_instance; }
    static bool isSupported(const QUrl& url);

protected:
    void runParser(const QUrl &url);
    
private slots:
    void parseOutput(void);

private:
    QProcess m_process;
    
    static ParserYkdl s_instance;
};

#endif // PARSERYKDL_H
