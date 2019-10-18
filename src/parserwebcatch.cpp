#include "parserwebcatch.h"
#include <QNetworkCookieJar>
#include <QNetworkReply>
#include <QWebEngineCookieStore>
#include <QWebEngineHttpRequest>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
#include <QWebEngineView>
#include "accessmanager.h"
#include "chromiumdebugger.h"
#include "extractor.h"

ParserWebCatch *parser_webcatch;

/* Init */
ParserWebCatch::ParserWebCatch(QObject *parent) :
    ParserBase(parent)
{
    // init extractors
    initExtractors();

    // set profile
    QWebEngineProfile *profile = QWebEngineProfile::defaultProfile();
    profile->setHttpUserAgent(DEFAULT_UA);
    profile->settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    connect(profile->cookieStore(), &QWebEngineCookieStore::cookieAdded, this, &ParserWebCatch::onCookieAdded);
    profile->cookieStore()->loadAllCookies();

    // create chromium instance
    webengineView = new QWebEngineView;
    webengineView->setUrl(QUrl("about:blank"));  // A trick to wait until QWebengine's initialization finishes

    // create debugger
    chromiumDebugger = new ChromiumDebugger(this);
    connect(chromiumDebugger, &ChromiumDebugger::connected, this, &ParserWebCatch::onChromiumConnected);
    connect(chromiumDebugger, &ChromiumDebugger::eventReceived, this, &ParserWebCatch::onChromiumEvent);
    connect(chromiumDebugger, &ChromiumDebugger::resultReceived, this, &ParserWebCatch::onChromiumResult);
    chromiumDebugger->open(19260);
}

void ParserWebCatch::onChromiumConnected()
{
    // enable network monitoring
    chromiumDebugger->send(1, "Network.enable");
}

/* Start parsing */
void ParserWebCatch::runParser(const QString &url)
{
    // Check if URL is supported
    if (!Extractor::isSupported(QUrl(url).host()))
    {
        showErrorDialog(tr("This URL is not supported now!"));
        return;
    }

    // load url
    QWebEngineHttpRequest request(url);
    request.setHeader("Referer", url.toUtf8());
    webengineView->setUrl(QUrl(url));
    webengineView->show();
}

/* Monitor network traffic */
void ParserWebCatch::onChromiumEvent(int id, const QString &method, const QVariantHash &params)
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
void ParserWebCatch::onChromiumResult(int id, const QVariantHash &result)
{
    Q_UNUSED(id);
    if (result.contains("body"))
    {
        QByteArray data = result["body"].toString().toUtf8();
        QString err = matchedExtractor->parse(data);
        if (!err.isEmpty())
            showErrorDialog(err);
        matchedExtractor = NULL;
    }
}

// finish parsing
void ParserWebCatch::onParseFinished(const QVariantHash &data)
{
    webengineView->setUrl(QUrl("about:blank"));
    webengineView->close();

    result.title = data["title"].toString();
    result.danmaku_url = data["danmaku_url"].toString();

    // read streams
    QVariantList streams = data["streams"].toList();
    QStringList stream_types;
    foreach (QVariant item, streams)
        stream_types << item.toHash()["type"].toString();

    // select video quality
    int selected = selectQuality(stream_types);
    if (selected == -1) // no item selected
        return;

    // Set source urls
    result.urls = streams[selected].toHash()["srcs"].toStringList();

    // find out container
    if (!result.urls.isEmpty())
        result.container = QUrl(result.urls[0]).path().section('.', -1);
    finishParsing();
}

/* Load cookies from QWebEngine */
void ParserWebCatch::onCookieAdded(const QNetworkCookie &cookie)
{
    access_manager->cookieJar()->insertCookie(cookie);
}
