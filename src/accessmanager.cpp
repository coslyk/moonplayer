#include "accessmanager.h"
#include "cookiejar.h"
#include "platform/paths.h"
#include "settings_network.h"
#include <QFile>
#include <QNetworkProxy>

const QString china_fake_ip = "220.181.111.63";
NetworkAccessManager *access_manager = 0;
QHash<QString,QByteArray> referer_table;
QHash<QString,QByteArray> ua_table;
QStringList unseekable_hosts;


// Generate a user-agent from URL
QByteArray generateUA(const QUrl &url)
{
    // custom UA
    QString host = url.host();
    if (ua_table.contains(host))
        return ua_table[host];
    return DEFAULT_UA;
}



NetworkAccessManager::NetworkAccessManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
    // Set cookie jar
    CookieJar *cookieJar = new CookieJar(this);
    setCookieJar(cookieJar);
}


void NetworkAccessManager::setProxy(const QString &proxyType, const QString &proxy, int port)
{
    if (proxyType == "no" || proxy.isEmpty())
    {
        // Set proxy for this access manager
        QNetworkAccessManager::setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
        // QWebEngine uses application proxy
        QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::NoProxy));
        // libmpv uses proxy from environment
        qunsetenv("http_proxy");
    }
    else if (proxyType == "socks5")
    {
        QNetworkAccessManager::setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, proxy, port));
        QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, proxy, port));
        qunsetenv("http_proxy");
    }
    else if (proxyType == "http")
    {
        QNetworkAccessManager::setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));
        QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));
        qputenv("http_proxy", QString("http://%1:%2").arg(proxy, QString::number(port)).toUtf8());
    }
}


QNetworkReply *NetworkAccessManager::get(const QNetworkRequest &req)
{
    // set user agent
    QNetworkRequest request = req;
    request.setHeader(QNetworkRequest::UserAgentHeader, generateUA(request.url()));
    return QNetworkAccessManager::get(request);
}

