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
    void runParser(const QUrl &url) override;

private slots:
    void parseOutput(void);

private:
    void runParserFull(const QUrl &url, bool parsePlaylist);
    void parseEpisode(QJsonObject episode);

    QProcess m_process;
    QUrl m_url;
    bool m_parsePlaylist;
    static ParserLux s_instance;
};

#endif // PARSERLUX_H
