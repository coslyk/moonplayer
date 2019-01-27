#include "simuparser.h"
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QWebEngineCookieStore>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include "accessmanager.h"
#include "chromiumdebugger.h"
#include "extractor.h"
#include "pyapi.h"

/* Initialization */
SimuParser::SimuParser(QObject *parent) :
    QObject(parent)
{
    // set profile
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    profile->setHttpUserAgent(DEFAULT_UA);
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
    if (method == "Network.responseReceived") // Check if url matches
    {
        QString url = params["response"].toHash()["url"].toString();
        id = -1;
        for (int i = 0; i < n_extractors; i++)
        {
            if (extractors[i]->match(url))
                id = i;
        }
        if (id != -1) // Url matches, start catching data
        {
            catchedUrl = url;
            catchedRequestId = params["requestId"].toString();
            selectedExtractor = id;
        }
    }
    else if (method == "Network.loadingFinished" && !catchedUrl.isEmpty()) // Catching finished, request body
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
    if (result.contains("body"))
    {
        QByteArray data = result["body"].toString().toUtf8();
        PyObject *result = extractors[selectedExtractor]->parse(data);
        if (result == NULL)
        {
            QString errString = QString("Python Exception:\n%1\n\nRequest URL: %2\n\nResponse Content:\n%3").arg(
                        fetchPythonException(), catchedUrl, QString::fromUtf8(data));
            emit parseError(errString);
        }
        else
            Py_DecRef(result);
        catchedUrl.clear();
    }
}

/* Load cookies from QWebEngine */
void SimuParser::onCookieAdded(const QNetworkCookie &cookie)
{
    access_manager->cookieJar()->insertCookie(cookie);
}
