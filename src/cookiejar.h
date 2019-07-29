#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include <QNetworkCookieJar>

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    explicit CookieJar(QObject *parent = NULL);
    bool exportNetscapeCookiesFile(const QString &filename);
};

#endif // COOKIEJAR_H
