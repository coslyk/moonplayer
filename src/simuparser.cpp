#include "simuparser.h"
#include <QNetworkCookieJar>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QWebSettings>
#include <QWebView>
#include "accessmanager.h"
#include "extractor.h"
#include "settings_network.h"
#include "utils.h"

SimuParser::SimuParser(QObject *parent) :
    QNetworkAccessManager(parent)
{
    initExtractors();

    // Use the same cookiejar
    QNetworkCookieJar *jar = access_manager->cookieJar();
    setCookieJar(jar);
    jar->setParent(access_manager);

    // Enable video loading
    QWebSettings *settings = QWebSettings::globalSettings();
    settings->setAttribute(QWebSettings::AutoLoadImages, false);
    settings->setAttribute(QWebSettings::PluginsEnabled, false);
    webview = new QWebView;
    webview->setWindowTitle(tr("Simulate web page loading"));
    webview->page()->setNetworkAccessManager(this);
}

void SimuParser::parse(const QString &url)
{
    // apply proxy settings
    if (Settings::proxyType == "http" || (Settings::proxyType == "http_unblockcn" && !url.contains(".youtube.com")))
        setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, Settings::proxy, Settings::port));
    else if (Settings::proxyType == "socks5")
        setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, Settings::proxy, Settings::port));
    else
        setProxy(QNetworkProxy::NoProxy);

    // load url
    webview->setUrl(QUrl(url));
    webview->show();
    webview->setWindowState(Qt::WindowMinimized);
}


// Watch the network traffic. If the URL of http request matches, catch the response and parse it
QNetworkReply *SimuParser::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    QNetworkRequest req = request;
    req.setHeader(QNetworkRequest::UserAgentHeader, DEFAULT_UA);

    // prevent video from loading
    QString suffix = req.url().path().section('.', -1);
    if (suffix == "mp4" || suffix == "flv" || suffix == "f4v" || suffix == "m3u8")
    {
        QUrl new_url = req.url();
        new_url.setHost("127.0.0.1");
        req.setUrl(new_url);
    }

    QNetworkReply *reply = QNetworkAccessManager::createRequest(op, req, outgoingData);

    // watch response if url matches
    QString url = req.url().toString();
    for (int i = 0; i < n_extractors; i++)
    {
        if (extractors[i]->match(url))
        {
            QByteArray *data = new QByteArray;
            connect(reply, &QNetworkReply::readyRead,
                    std::bind(&SimuParser::onReadyRead, this, reply, data));
            connect(reply, &QNetworkReply::finished,
                    std::bind(&SimuParser::onFinished, this, reply, extractors[i], data));
        }
    }
    return reply;
}

void SimuParser::onReadyRead(QNetworkReply *reply, QByteArray *data)
{
    data->append(reply->readAll());
}

void SimuParser::onFinished(QNetworkReply *reply, Extractor *extractor, QByteArray *data)
{
    data->append(reply->readAll());
    PyObject *result = extractor->parse(*data);
    if (result == NULL)
    {

        PyObject *ptype, *pvalue, *ptraceback;
        PyErr_Fetch(&ptype, &pvalue, &ptraceback);
        QString errString = QString("Python Exception: %1\n\nURL: %2\n\nResponse Content:\n%3").arg(
                    PyString_AsQString(pvalue), reply->url().toString(), QString::fromUtf8(*data));
        Py_DecRef(ptype);
        Py_DecRef(pvalue);
        Py_DecRef(ptraceback);
        emit parseError(errString);
    }
    else
    {
        webview->setUrl(QUrl("about:blank"));
        webview->close();
        emit parseFinished(result);
        Py_DecRef(result);
    }
    delete data;
}
