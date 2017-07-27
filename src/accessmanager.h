#ifndef ACCESSMANAGER_H
#define ACCESSMANAGER_H

#include <QHash>
#include <QUrl>

class QNetworkAccessManager;
extern QNetworkAccessManager *access_manager;
extern QStringList unseekable_hosts;
extern QHash<QString,QByteArray> referer_table;
extern QHash<QString,QByteArray> ua_table;
QByteArray generateUA(const QUrl &url);

#endif // NETWORKMANAGER_H
