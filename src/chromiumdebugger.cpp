#include "chromiumdebugger.h"
#include "accessmanager.h"
#include <QJsonDocument>
#include <QMessageBox>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QWebSocket>

ChromiumDebugger::ChromiumDebugger(QObject *parent) :
    QObject(parent)
{
    ws = new QWebSocket("MoonPlayer", QWebSocketProtocol::VersionLatest, parent);
    connect(ws, &QWebSocket::textMessageReceived, this, &ChromiumDebugger::onReceived);
    connect(ws, &QWebSocket::connected, this, &ChromiumDebugger::onConnected);
    connect(ws, &QWebSocket::disconnected, this, &ChromiumDebugger::onDisconnected);
}

// establish connection
void ChromiumDebugger::open(int port)
{
    QString addr = "http://localhost:" + QString::number(port) + "/json";
    reply = access_manager->get(QNetworkRequest(addr));
    connect(reply, &QNetworkReply::finished, this, &ChromiumDebugger::onDebugInfoReceived);
}

void ChromiumDebugger::onDebugInfoReceived()
{
    if (reply->error() == QNetworkReply::NoError)
    {
        QString url = QJsonDocument::fromJson(reply->readAll()).toVariant().toList()[0].toHash()["webSocketDebuggerUrl"].toString();
        ws->open(QUrl(url));
    }
    else
        QMessageBox::warning(nullptr, "Error", "Fails to get Chromium's debug info!");
    reply->deleteLater();
}

void ChromiumDebugger::send(int id, const QString &method, const QVariantHash &params)
{
    QVariantHash data;
    data["id"] = id;
    data["method"] = method;
    data["params"] = params;
    QByteArray jsonData = QJsonDocument::fromVariant(data).toJson(QJsonDocument::Compact);
    ws->sendTextMessage(QString::fromUtf8(jsonData));
}

void ChromiumDebugger::onReceived(const QString &message)
{
    QVariantHash data = QJsonDocument::fromJson(message.toUtf8()).toVariant().toHash();
    int id = data["id"].toInt();
    if (data.contains("method") && data.contains("params")) // event
    {
        QString method = data["method"].toString();
        QVariantHash params = data["params"].toHash();
        emit eventReceived(id, method, params);
    }
    else if (data.contains("result")) // return of a call
    {
        QVariantHash result = data["result"].toHash();
        emit resultReceived(id, result);
    }
}

void ChromiumDebugger::onConnected()
{
    qDebug("[ChromiumDebugger] connected");
    emit connected();
}

void ChromiumDebugger::onDisconnected()
{
    qDebug("[ChromiumDebugger] Chromium disconnected");
}
