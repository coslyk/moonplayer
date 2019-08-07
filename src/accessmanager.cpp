#include "accessmanager.h"
#include "platform/paths.h"
#include "proxyfactory.h"
#include "settings_network.h"
#include <QFile>
#include <QNetworkProxy>


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
    m_proxyFactory = new ProxyFactory;
    m_webengineProxyFactory = new ProxyFactory;

    setProxyFactory(m_proxyFactory);

    // QWebEngine uses application proxy
    // QWebEngine in MoonPlayer is used only for parsing videos
    QNetworkProxyFactory::setApplicationProxyFactory(m_webengineProxyFactory);
}


void NetworkAccessManager::setProxy(const QString &proxyType, const QString &proxy, int port)
{
    if (proxyType == "no" || proxy.isEmpty())
    {
        m_proxyFactory->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
        m_webengineProxyFactory->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
        qunsetenv("http_proxy");    // libmpv uses proxy from environment
    }
    else if (proxyType == "socks5")
    {
        if (Settings::proxyOnlyForParsing)
            m_proxyFactory->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
        else
            m_proxyFactory->setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, proxy, port));
        qunsetenv("http_proxy");    // libmpv does not support socks5 yet
        m_webengineProxyFactory->setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, proxy, port));
    }
    else if (proxyType == "http")
    {
        if (Settings::proxyOnlyForParsing)
        {
            m_proxyFactory->setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
            qunsetenv("http_proxy");
        }
        else
        {
            m_proxyFactory->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));
            qputenv("http_proxy", QString("http://%1:%2").arg(proxy, QString::number(port)).toUtf8());
        }
        m_webengineProxyFactory->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));
    }
}


QNetworkReply *NetworkAccessManager::get(const QNetworkRequest &req)
{
    // set user agent
    QNetworkRequest request = req;
    request.setHeader(QNetworkRequest::UserAgentHeader, generateUA(request.url()));
    return QNetworkAccessManager::get(request);
}

