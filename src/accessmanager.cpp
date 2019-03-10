#include "accessmanager.h"
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
    amForUnblock = nullptr;
    // read header urls list
    QString header_urls_file = getAppPath() + "/unblockcn/header_urls.txt";
    if (QFile::exists(header_urls_file))
    {
        QFile f(header_urls_file);
        if (f.open(QFile::ReadOnly | QFile::Text))
        {
            do
            {
                QString line = QString::fromUtf8(f.readLine().simplified());
                if (!line.isEmpty())
                    header_urls << line;
            } while (!f.atEnd());
            f.close();
        }
    }
    // read proxy urls list
    QString proxy_urls_file = getAppPath() + "/unblockcn/proxy_urls.txt";
    if (QFile::exists(proxy_urls_file))
    {
        QFile f(proxy_urls_file);
        if (f.open(QFile::ReadOnly | QFile::Text))
        {
            do
            {
                QString line = QString::fromUtf8(f.readLine().simplified());
                if (!line.isEmpty())
                    proxy_urls << line;
            } while (!f.atEnd());
            f.close();
        }
    }
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
    else if (proxyType == "http_unblockcn")
    {
        // this access manager does not use proxy
        QNetworkAccessManager::setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
        // QWebEngine uses proxy to parse videos
        QNetworkProxy::setApplicationProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));
        // libmpv does not use proxy
        qunsetenv("http_proxy");
        // access manager for unblocking
        if (amForUnblock == nullptr)
            amForUnblock = new QNetworkAccessManager(this);
        amForUnblock->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));
    }
}


QNetworkReply *NetworkAccessManager::get(const QNetworkRequest &req)
{
    // set user agent
    QNetworkRequest request = req;
    QString url = request.url().toString();
    request.setHeader(QNetworkRequest::UserAgentHeader, generateUA(request.url()));

    // unblock mode is disable
    if (Settings::proxyType != "http_unblockcn")
        return QNetworkAccessManager::get(request);

    // unblock mode is enable
    // check if the website can be unblocked by modifying headers
    foreach (QString patt, header_urls)
    {
        if (url.contains(patt))
        {
            qInfo("[UnblockCN] Fake ip for: %s", url.toUtf8().constData());
            request.setRawHeader("X-Forwarded-For", china_fake_ip.toUtf8());
            request.setRawHeader("Client-IP", china_fake_ip.toUtf8());
            return QNetworkAccessManager::get(request);
        }
    }

    // check if the website can be unblocked by proxy
    foreach (QString patt, proxy_urls)
    {
        if (url.contains(patt))
        {
            qInfo("[UnblockCN] Use proxy for: %s", url.toUtf8().constData());
            return amForUnblock->get(request);
        }
    }
    qInfo("[UnblockCN] access directly for: %s", url.toUtf8().constData());
    return QNetworkAccessManager::get(request);
}

