#ifndef PARSERWEBCATCH_H
#define PARSERWEBCATCH_H

#include "parserbase.h"
class ChromiumDebugger;
class Extractor;
class QNetworkCookie;
class QWebEngineView;

class ParserWebCatch : public ParserBase
{
    Q_OBJECT
public:
    ParserWebCatch(QObject *parent = 0);
    void onParseFinished(const QVariantHash &data);

protected:
    void runParser(const QString &url);

private slots:
    void onChromiumConnected(void);
    void onChromiumEvent(int id, const QString &method, const QVariantHash &params);
    void onChromiumResult(int id, const QVariantHash &result);
    void onCookieAdded(const QNetworkCookie &cookie);

private:
    Extractor *matchedExtractor;
    QString catchedRequestId;
    QWebEngineView *webengineView;
    ChromiumDebugger *chromiumDebugger;
};

extern ParserWebCatch *parser_webcatch;

#endif // PARSERWEBCATCH_H
