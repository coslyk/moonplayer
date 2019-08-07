#ifndef ACCESSMANAGER_H
#define ACCESSMANAGER_H

#include <QHash>
#include <QNetworkAccessManager>
#include <QUrl>

class ProxyFactory;

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    NetworkAccessManager(QObject *parent = NULL);
    QNetworkReply *get(const QNetworkRequest &request);
    void setProxy(const QString &proxyType, const QString &proxy, int port);

private:
    ProxyFactory *m_proxyFactory;
    ProxyFactory *m_webengineProxyFactory;
};

extern NetworkAccessManager *access_manager;
extern QStringList unseekable_hosts;
extern QHash<QString,QByteArray> referer_table;
extern QHash<QString,QByteArray> ua_table;
QByteArray generateUA(const QUrl &url);

#define DEFAULT_UA \
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko)"\
    "Ubuntu Chromium/72.0.3626.28 Chrome/72.0.3626.28 Safari/537.36"

#endif // NETWORKMANAGER_H
