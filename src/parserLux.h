#ifndef PARSERLUX_H
#define PARSERLUX_H

#include "parserBase.h"
#include <QProcess>

class ParserLux : public ParserBase
{
    Q_OBJECT
public:
    explicit ParserLux(QObject *parent = nullptr);
    ~ParserLux();
    inline static ParserLux* instance() { return &s_instance; }

protected:
    void runParser(const QUrl &url);

private slots:
    void parseOutput(void);

private:
    QProcess m_process;
    static ParserLux s_instance;
};

#endif // PARSERLUX_H
