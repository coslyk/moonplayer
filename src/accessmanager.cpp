#include "accessmanager.h"
#include "platforms.h"
#include "settings_network.h"
#include <QFile>
#include <QNetworkProxy>
#include <QProcess>

const QString china_fake_ip = "220.181.111.63";

NetworkAccessManager::NetworkAccessManager(QObject *parent) :
    QNetworkAccessManager(parent)
{
    amForUnblock = NULL;
    // read header urls list
    QString header_urls_file = getUserPath() + "/unblockcn/header_urls.txt";
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
    QString proxy_urls_file = getUserPath() + "/unblockcn/proxy_urls.txt";
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


QNetworkReply *NetworkAccessManager::get(const QNetworkRequest &req)
{
    // unblock mode is disable
    if (Settings::proxyType != "http_unblockcn")
        return QNetworkAccessManager::get(req);

    // unblock mode is enable
    QNetworkRequest request = req;
    QString url = request.url().toString();

    // check if the website can be unblocked by modifying headers
    foreach (QString patt, header_urls)
    {
        if (url.contains(patt))
        {
            request.setRawHeader("X-Forwarded-For", china_fake_ip.toUtf8());
            request.setRawHeader("Client-IP", china_fake_ip.toUtf8());
            return QNetworkAccessManager::get(request);
        }
    }

    // check if the website can be unblocked by proxy
    if (amForUnblock == NULL)
        amForUnblock = new QNetworkAccessManager(this);
    amForUnblock->setProxy(QNetworkProxy(QNetworkProxy::HttpProxy, Settings::proxy, Settings::port));
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


NetworkAccessManager *access_manager = 0;
QHash<QString,QByteArray> referer_table;
QHash<QString,QByteArray> ua_table;
QStringList unseekable_hosts;

QByteArray generateUA(const QUrl &url)
{
    static QByteArray default_ua;

    QString host = url.host();
    if (ua_table.contains(host))
        return ua_table[host];

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
