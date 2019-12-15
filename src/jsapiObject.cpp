#include "jsapiObject.h"
#include "accessManager.h"
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>

JSAPIObject::JSAPIObject(QObject* parent) : QObject(parent)
{
}


void JSAPIObject::get_post_content(const QString& url, const QByteArray& postData, const QJSValue& callbackFunc)
{
    QNetworkRequest request = QNetworkRequest(url);
    QNetworkReply *reply;
    if (postData.isEmpty())
        reply = NetworkAccessManager::instance()->get(request);
    else
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
        reply = NetworkAccessManager::instance()->post(request, postData);
    }

    QObject::connect(reply, &QNetworkReply::finished, [=](){
        reply->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        // redirection
        if (status == 301 || status == 302)
        {
            QByteArray final_url = reply->rawHeader("Location");
            get_post_content(QString::fromUtf8(final_url), postData, callbackFunc);
            return;
        }

        // Error
        if (reply->error() != QNetworkReply::NoError)
        {
            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString errStr = QString().sprintf("Network Error: %d\n%s\n", status, reply->errorString().toUtf8().constData());
            QMessageBox::warning(NULL, "Error", errStr);
            return;
        }

        // Call callback function
        QJSValueList args;
        args << QString::fromUtf8(reply->readAll());
        QJSValue func = callbackFunc;
        QJSValue retVal = func.call(args);
        if (retVal.isError())
            emit jsError(retVal);
    });
}

void JSAPIObject::get_content(const QString& url, const QJSValue& callbackFunc)
{
    get_post_content(url, QByteArray(), callbackFunc);
}

void JSAPIObject::post_content(const QString& url, const QByteArray& postData, const QJSValue& callbackFunc)
{
    get_post_content(url, postData, callbackFunc);
}

void JSAPIObject::warning(const QString& msg)
{
    QMessageBox::warning(nullptr, "Warning", msg);
}

bool JSAPIObject::question(const QString& msg)
{
    return QMessageBox::question(nullptr, "Question", msg, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
}

void JSAPIObject::show_result(const QVariant& result)
{
    emit showResultRequested(result);
}
