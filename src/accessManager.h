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
    Q_ENUMS(ProxyType);
    
    NetworkAccessManager(QObject *parent = nullptr);
    
    static NetworkAccessManager *instance(void); // Singleton
    
    QNetworkReply *get(const QNetworkRequest &request);
    
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
