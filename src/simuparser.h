#ifndef SimuParser_H
#define SimuParser_H

#include <QNetworkAccessManager>
#include <Python.h>
class QWebView;
class Extractor;

class SimuParser : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit SimuParser(QObject *parent = 0);
    void parse(const QString &url);

signals:
    void parseFinished(PyObject *o);
    void parseError(const QString &errMsg);

protected:
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData);

private:
    void onReadyRead(QNetworkReply *reply, QByteArray *data);
    void onFinished(QNetworkReply *reply, Extractor *extractor, QByteArray *data);

    QWebView *webview;
};

#endif // SimuParser_H
