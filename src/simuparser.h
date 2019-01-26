#ifndef SimuParser_H
#define SimuParser_H

#include <Python.h>
#include <QWebEngineView>
class ChromiumDebugger;
class Extractor;

class SimuParser : public QObject
{
    Q_OBJECT
public:
    explicit SimuParser(QObject *parent = 0);
    void parse(const QString &url);
    inline void closeWebview() {
        webengineView->setUrl(QUrl("about:blank"));
        webengineView->close();
    }

signals:
    void parseError(const QString &errMsg);

private:
    void onChromiumConnected(void);
    void onChromiumEvent(int id, const QString &method, const QVariantHash &params);
    void onChromiumResult(int id, const QVariantHash &result);

    int selectedExtractor;
    QString catchedUrl;
    QString catchedRequestId;
    QWebEngineView *webengineView;
    ChromiumDebugger *chromiumDebugger;
};

#endif // SimuParser_H
