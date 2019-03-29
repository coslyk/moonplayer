#ifndef CHROMIUMDEBUGGER_H
#define CHROMIUMDEBUGGER_H

#include <QObject>
#include <QVariantHash>
class QNetworkReply;
class QWebSocket;

// Communicate with chromium's remote debugger
class ChromiumDebugger : public QObject
{
    Q_OBJECT
public:
    ChromiumDebugger(QObject *parent = NULL);
    void open(int port);
    void send(int id, const QString &method, const QVariantHash &params = QVariantHash());

signals:
    void connected(void);
    void eventReceived(int id, const QString &method, const QVariantHash &params);
    void resultReceived(int id, const QVariantHash &result);

private slots:
    void onDebugInfoReceived(void);
    void onConnected(void);
    void onDisconnected(void);
    void onReceived(const QString &message);

private:
    QWebSocket *ws;
    QNetworkReply *reply;
};

#endif // CHROMIUMDEBUGGER_H
