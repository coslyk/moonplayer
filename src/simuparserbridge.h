#ifndef SIMUPARSERBRIDGE_H
#define SIMUPARSERBRIDGE_H

#include "parserbridge.h"
class ChromiumDebugger;
class Extractor;
class QNetworkCookie;
class QWebEngineView;

class SimuParserBridge : public ParserBridge
{
    Q_OBJECT
public:
    SimuParserBridge(QObject *parent = 0);
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

extern SimuParserBridge *simuParserBridge;

#endif // SIMUPARSERBRIDGE_H
