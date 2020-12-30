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

#ifndef ACCESSMANAGER_H
#define ACCESSMANAGER_H

// Manage network access of MoonPlayer

#include <QHash>
#include <QNetworkAccessManager>
#include <QUrl>


#define DEFAULT_UA \
    "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko)"\
    "Ubuntu Chromium/72.0.3626.28 Chrome/72.0.3626.28 Safari/537.36"

class ProxyFactory;

class NetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    enum ProxyType {NO_PROXY, HTTP_PROXY, SOCKS5_PROXY};
    Q_ENUM(ProxyType)
    
    NetworkAccessManager(QObject *parent = nullptr);
    
    static NetworkAccessManager *instance(void); // Singleton
    
    QNetworkReply *get(const QNetworkRequest &request);
    QNetworkReply *post(const QNetworkRequest &request, const QByteArray &data);

    Q_INVOKABLE void setupProxy(ProxyType proxyType, const QString& proxy = QString(), bool proxyOnlyForParsing = false);
    
    inline void addUnseekableHost(const QString& host) { m_unseekableHosts << host; }
    inline bool urlIsUnseekable(const QUrl& url) { return m_unseekableHosts.contains(url.host()); }
    
    inline void addReferer(const QUrl& url, const QByteArray& referer) { m_refererTable[url.host()] = referer; }
    inline QByteArray refererOf(const QUrl& url) { return m_refererTable[url.host()]; }
    
    inline void addUserAgent(const QUrl& url, const QByteArray& ua) { m_ua_table[url.host()] = ua; }
    inline QByteArray userAgentOf(const QUrl& url) { return m_ua_table[url.host()].isEmpty() ? DEFAULT_UA : m_ua_table[url.host()]; }
    
private:
    ProxyFactory *m_proxyFactory;
    QStringList m_unseekableHosts;
    QHash<QString,QByteArray> m_refererTable;
    QHash<QString,QByteArray> m_ua_table;
};


#endif // NETWORKMANAGER_H
