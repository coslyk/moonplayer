#ifndef PROXYFACTORY_H
#define PROXYFACTORY_H

#include <QNetworkProxyFactory>

class ProxyFactory : public QNetworkProxyFactory
{
public:
    explicit ProxyFactory(void);
    void setProxy(const QNetworkProxy &proxy);
    virtual QList<QNetworkProxy> queryProxy(const QNetworkProxyQuery &query);

private:
    QNetworkProxy m_proxy;
};

#endif // PROXYFACTORY_H
