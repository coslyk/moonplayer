#include "simuparser.h"
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>
#include "accessmanager.h"
#include "chromiumdebugger.h"
#include "extractor.h"

/* Initialization */
SimuParser::SimuParser(QObject *parent) :
    QObject(parent)
{
    // set profile
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    profile->setHttpUserAgent(DEFAULT_UA);
    profile->settings()->setAttribute(QWebEngineSettings::AutoLoadImages, false);
    profile->settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    connect(profile->cookieStore(), &QWebEngineCookieStore::cookieAdded, this, &SimuParser::onCookieAdded);
    profile->cookieStore()->loadAllCookies();

    // create chromium instance
    webengineView = new QWebEngineView;
    webengineView->setUrl(QUrl("about:blank"));  // A trick to wait until QWebengine's initialization finishes

    // create debugger
    chromiumDebugger = new ChromiumDebugger(this);
    connect(chromiumDebugger, &ChromiumDebugger::connected, this, &SimuParser::onChromiumConnected);
    connect(chromiumDebugger, &ChromiumDebugger::eventReceived, this, &SimuParser::onChromiumEvent);
    connect(chromiumDebugger, &ChromiumDebugger::resultReceived, this, &SimuParser::onChromiumResult);
    chromiumDebugger->open(19260);
}

void SimuParser::onChromiumConnected()
{
    // enable network monitoring
    chromiumDebugger->send(1, "Network.enable");
}


/* Parse video*/
void SimuParser::parse(const QString &url)
{
    // Check if URL is supported
    if (!Extractor::isSupported(QUrl(url).host()))
    {
        emit parseError(tr("This URL is not supported now!"));
        return;
    }

    // load url
    webengineView->setUrl(QUrl(url));
    webengineView->show();
}

/* Monitor network traffic */
void SimuParser::onChromiumEvent(int id, const QString &method, const QVariantHash &params)
{
    Q_UNUSED(id);
    if (method == "Network.responseReceived") // Check if url matches
    {
        QString url = params["response"].toHash()["url"].toString();
        Extractor *match = Extractor::getMatchedExtractor(url);
        if (match)
        {
            matchedExtractor = match;
            catchedRequestId = params["requestId"].toString();
        }
    }
    else if (method == "Network.loadingFinished" && matchedExtractor) // Catching finished, request body
    {
        QString requestId = params["requestId"].toString();
        if (requestId == catchedRequestId)
        {
            QVariantHash newParams;
            newParams["requestId"] = catchedRequestId;
            chromiumDebugger->send(1, "Network.getResponseBody", newParams);
        }
    }
}

// read body
void SimuParser::onChromiumResult(int id, const QVariantHash &result)
{
    Q_UNUSED(id);
    if (result.contains("body"))
    {
        QByteArray data = result["body"].toString().toUtf8();
        QString err = matchedExtractor->parse(data);
        if (!err.isEmpty())
            emit parseError(err);
        matchedExtractor = nullptr;
    }
}

/* Load cookies from QWebEngine */
void SimuParser::onCookieAdded(const QNetworkCookie &cookie)
{
    access_manager->cookieJar()->insertCookie(cookie);
}
