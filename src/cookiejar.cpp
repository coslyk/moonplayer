#include "cookiejar.h"
#include <QFile>
#include <QNetworkCookie>

CookieJar::CookieJar(QObject *parent)
    : QNetworkCookieJar(parent)
{
    load();
}

CookieJar::~CookieJar()
{
    save();
}


void CookieJar::load()
{
    QFile file(cookieFileName());
    if (file.open(QFile::ReadOnly | QFile::Text))
    {
        QByteArray data = file.readAll();
        setAllCookies(QNetworkCookie::parseCookies(data));
        file.close();
    }
}


void CookieJar::save()
{
    QList<QNetworkCookie> list = allCookies();
    QByteArray data;
    foreach (QNetworkCookie cookie, list)
    {
        if (!cookie.isSessionCookie()) {
            data.append(cookie.toRawForm());
            data.append("\n");
        }
    }
    QFile file(cookieFileName());
    if (file.open(QFile::WriteOnly | QFile::Text))
    {
        file.write(data);
        file.close();
    }
}
