#include "proxyfactory.h"

ProxyFactory::ProxyFactory() :
    QNetworkProxyFactory(), m_proxy(QNetworkProxy::NoProxy)
{

}

void ProxyFactory::setProxy(const QNetworkProxy &proxy)
{
    m_proxy = proxy;
}

QList<QNetworkProxy> ProxyFactory::queryProxy(const QNetworkProxyQuery &query)
{
    QString host = query.peerHostName();
    QList<QNetworkProxy> proxies;
    if (host == "localhost" || host == "ip6-localhost" || host == "ip6-loopback" || host == "127.0.0.1" || host == "::1")
        proxies << QNetworkProxy(QNetworkProxy::NoProxy);
    else
        proxies << m_proxy;
    return proxies;
}
