#include "cookiejar.h"
#include <QDateTime>
#include <QFile>
#include <QNetworkCookie>
#include "platform/paths.h"

CookieJar::CookieJar(QObject *parent) :
    QNetworkCookieJar(parent)
{
}

bool CookieJar::exportNetscapeCookiesFile(const QString &filename)
{
    QList<QNetworkCookie> cookies = allCookies();
    QString content;

    foreach (QNetworkCookie cookie, cookies) {
        // convert to mozilla's format
        QDateTime expirationDate = cookie.expirationDate();
        content += QString("%1\tTRUE\t%2\t%3\t%4\t%5\t%6\n").arg(
                    cookie.domain(),
                    cookie.path(),
                    cookie.isSecure() ? "TRUE" : "FALSE",
                    expirationDate.isValid() ? QString::number(expirationDate.toSecsSinceEpoch()) : "",
                    QString::fromUtf8(cookie.name()),
                    QString::fromUtf8(cookie.value())
                    );
    }
    if (content.isEmpty())  //no cookies needed
        return false;
    else
    {
        QFile file(filename);
        if (!file.open(QFile::WriteOnly | QFile::Text))
            return false;
        file.write("# Netscape HTTP Cookie File\n"
           "# http://curl.haxx.se/rfc/cookie_spec.html\n"
           "# This is a generated file!  Do not edit.\n\n");
        file.write(content.toUtf8());
        file.close();
        return true;
    }
}
