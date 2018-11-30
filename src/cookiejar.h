#ifndef COOKIEJAR_H
#define COOKIEJAR_H

#include <QNetworkCookieJar>
#include "platforms.h"

class CookieJar : public QNetworkCookieJar
{
    Q_OBJECT
public:
    CookieJar(QObject *parent = 0);
    ~CookieJar();
    void load();
    void save();
    inline QString cookieFileName() { return getUserPath() + "/cookies.txt"; }
};

#endif // COOKIEJAR_H
