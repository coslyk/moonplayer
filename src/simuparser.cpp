#include "simuparser.h"
#include <QNetworkReply>
#include <QWebSettings>
#include <QWebView>
#include "extractor.h"

SimuParser::SimuParser(QObject *parent) :
    QNetworkAccessManager(parent)
{
    initExtractors();

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
    // load url
    webview->setUrl(QUrl(url));
    webview->show();
    webview->setWindowState(Qt::WindowMinimized);
}


// Watch the network traffic. If the URL of http request matches, catch the response and parse it
QNetworkReply *SimuParser::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    QNetworkRequest req = request;
    req.setHeader(QNetworkRequest::UserAgentHeader,
                  "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) "
                  "Ubuntu Chromium/71.0.3578.20 "
                  "Chrome/71.0.3578.20 Safari/537.36");

    // prevent video from loading
    QString suffix = req.url().path().section('.', -1);
    if (suffix == "mp4" || suffix == "flv" || suffix == "f4v" || suffix == "m3u8")
    {
        QUrl new_url = req.url();
        new_url.setHost("127.0.0.1");
        req.setUrl(new_url);
    }

    // watch response if url matches
    QNetworkReply *reply = QNetworkAccessManager::createRequest(op, req, outgoingData);
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
    delete data;
    if (result == NULL)
        PyErr_Print();
    else
    {
        emit parseFinished(result);
        Py_DecRef(result);
    }
}
