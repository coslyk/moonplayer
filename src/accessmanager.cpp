#include "accessmanager.h"
#include "platforms.h"
#include "settings_network.h"
#include <QFile>
#include <QNetworkProxy>
#include <QProcess>

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

    // default UA
    static QByteArray default_ua;
    if (default_ua.isNull())
    {
        QStringList args;
        args << "-c" << "import sys; print('Python-urllib/%d.%d' % sys.version_info[:2])";
        QProcess process;
        if (QFile::exists("/usr/bin/python3"))
            process.start("/usr/bin/python3", args, QProcess::ReadOnly);
        else if (QFile::exists("/usr/local/bin/python3"))
            process.start("/usr/local/bin/python3", args, QProcess::ReadOnly);
        else
        {
            default_ua = "moonplayer";
            return default_ua;
        }
        process.waitForFinished();
        default_ua = process.readAll().simplified();
    }
    return default_ua;
}



NetworkAccessManager::NetworkAccessManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
    amForUnblock = NULL;
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
        QNetworkAccessManager::setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
        unsetenv("http_proxy");
    }
    else if (proxyType == "socks5")
    {
        QNetworkAccessManager::setProxy(QNetworkProxy(QNetworkProxy::Socks5Proxy, proxy, port));
        unsetenv("http_proxy");
    }
    else if (proxyType == "http")
    {
        QNetworkAccessManager::setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, proxy, port));
        setenv("http_proxy", QString("http://%1:%2").arg(proxy, QString::number(port)).toUtf8().constData(), 1);
    }
    else if (proxyType == "http_unblockcn")
    {
        QNetworkAccessManager::setProxy(QNetworkProxy(QNetworkProxy::NoProxy));
        unsetenv("http_proxy");
        if (amForUnblock == NULL)
            amForUnblock = new QNetworkAccessManager(this);
        amForUnblock->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, Settings::proxy, Settings::port));
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

