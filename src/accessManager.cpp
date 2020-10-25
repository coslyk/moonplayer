/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#include "accessManager.h"
#include <QNetworkProxy>


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
        if (host == QStringLiteral("localhost") ||
            host == QStringLiteral("ip6-localhost") ||
            host == QStringLiteral("ip6-loopback") ||
            host == QStringLiteral("127.0.0.1") ||
            host == QStringLiteral("::1"))
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
        request.setRawHeader(QByteArrayLiteral("Referer"), refererOf(request.url()));
    return QNetworkAccessManager::get(request);
}


void NetworkAccessManager::setupProxy(NetworkAccessManager::ProxyType proxyType, const QString& proxy, bool proxyOnlyForParsing)
{
    QString ip = proxy.section(QLatin1Char(':'), 0, 0);
    int port = proxy.section(QLatin1Char(':'), -1).toInt();
    
    // Setup proxy
    if (proxyType == NO_PROXY || ip.isEmpty() || proxyOnlyForParsing)
    {
        m_proxyFactory->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
        qunsetenv("http_proxy");    // libmpv uses proxy from environment
        qunsetenv("https_proxy");
    }
    else if (proxyType == SOCKS5_PROXY)
    {
        m_proxyFactory->setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, ip, port));
        qunsetenv("http_proxy");    // libmpv does not support socks5 yet
        qunsetenv("https_proxy");
    }
    else
    {
        m_proxyFactory->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, ip, port));
        QByteArray proxy_str = (QStringLiteral("http://") + proxy).toUtf8();
        qputenv("http_proxy", proxy_str);
        qputenv("https_proxy", proxy_str);
    }
}
