#include "jsapiObject.h"
#include "accessManager.h"
#include <QInputDialog>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QSettings>

JSAPIObject::JSAPIObject(const QString &id, QObject* parent) : QObject(parent), m_id(id)
{
}


void JSAPIObject::get_post_content(const QString& url, const QByteArray& postData, const QJSValue& callbackFunc)
{
    Q_ASSERT(NetworkAccessManager::instance() != nullptr);

    QNetworkRequest request = QNetworkRequest(url);
    QNetworkReply *reply;
    if (postData.isEmpty())
        reply = NetworkAccessManager::instance()->get(request);
    else
    {
        request.setHeader(QNetworkRequest::ContentTypeHeader, QByteArrayLiteral("application/x-www-form-urlencoded"));
        reply = NetworkAccessManager::instance()->post(request, postData);
    }

    QObject::connect(reply, &QNetworkReply::finished, [=](){
        reply->deleteLater();
        int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        // redirection
        if (status == 301 || status == 302)
        {
            QByteArray final_url = reply->rawHeader(QByteArrayLiteral("Location"));
            get_post_content(QString::fromUtf8(final_url), postData, callbackFunc);
            return;
        }

        // Error
        if (reply->error() != QNetworkReply::NoError)
        {
            int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            QString errStr = QStringLiteral("Network Error: %1\n%2\n").arg(QString::number(status), reply->errorString());
            QMessageBox::warning(NULL, tr("Error"), errStr);
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

// Dialogs
void JSAPIObject::warning(const QString& msg)
{
    QMessageBox::warning(nullptr, tr("Warning"), msg);
}

bool JSAPIObject::question(const QString& msg)
{
    return QMessageBox::question(nullptr, tr("Question"), msg, QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes;
}

QString JSAPIObject::get_text(const QString &msg)
{
    return QInputDialog::getText(nullptr, tr("Please input"), msg);
}

QString JSAPIObject::get_item(const QString &msg, const QStringList &items)
{
    return QInputDialog::getItem(nullptr, tr("Please select"), msg, items);
}

// Show result
void JSAPIObject::show_result(const QVariant& result)
{
    emit showResultRequested(result);
}

// Configurations
QVariant JSAPIObject::get_configuration(const QString &name)
{
    QString key = QStringLiteral("plugin-%1/%2").arg(m_id, name);
    return QSettings().value(key);
}

void JSAPIObject::set_configuration(const QString &name, const QVariant &value)
{
    QString key = QStringLiteral("plugin-%1/%2").arg(m_id, name);
    QSettings().setValue(key, value);
}
