#include "accessManager.h"
#include <QCoreApplication>
#include <QFile>
#include <QNetworkProxy>
#include <QSettings>


// Proxy factory
class ProxyFactory : public QNetworkProxyFactory
{
    QNetworkProxy m_proxy;
public:
    void setProxy(const QNetworkProxy& proxy)
    {
        m_proxy = proxy;
    }
    virtual QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query)
    {
        // Ignore localhost
        QList<QNetworkProxy> proxies;
        QString host = query.peerHostName();
        if (host == "localhost" || host == "ip6-localhost" || host == "ip6-loopback" || host == "127.0.0.1" || host == "::1")
            proxies << QNetworkProxy(QNetworkProxy::NoProxy);
        else
            proxies << m_proxy;
        return proxies;
    }
};



NetworkAccessManager::NetworkAccessManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
    m_proxyFactory = new ProxyFactory;
    setProxyFactory(m_proxyFactory);
    QNetworkProxyFactory::setApplicationProxyFactory(m_proxyFactory);
}


NetworkAccessManager * NetworkAccessManager::instance()
{
    static NetworkAccessManager *singleton = nullptr;
    if (!singleton)
        singleton = new NetworkAccessManager;
    return singleton;
}



QNetworkReply *NetworkAccessManager::get(const QNetworkRequest &req)
{
    // set user agent
    QNetworkRequest request = req;
    request.setHeader(QNetworkRequest::UserAgentHeader, userAgentOf(request.url()));
    if (!refererOf(request.url()).isEmpty())
        request.setRawHeader("Referer", refererOf(request.url()));
    return QNetworkAccessManager::get(request);
}


void NetworkAccessManager::setupProxy(NetworkAccessManager::ProxyType proxyType, const QString& proxy, bool proxyOnlyForParsing)
{
    QString ip = proxy.section(':', 0, 0);
    int port = proxy.section(':', -1).toInt();
    
    // Setup proxy
    if (proxyType == NO_PROXY || ip.isEmpty() || proxyOnlyForParsing)
    {
        m_proxyFactory->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
        qunsetenv("http_proxy");    // libmpv uses proxy from environment
    }
    else if (proxyType == SOCKS5_PROXY)
    {
        m_proxyFactory->setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, ip, port));
        qunsetenv("http_proxy");    // libmpv does not support socks5 yet
    }
    else
    {
        m_proxyFactory->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, ip, port));
        qputenv("http_proxy", ("http://" + proxy).toUtf8());
    }
}
