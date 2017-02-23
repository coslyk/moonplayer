#ifndef ACCESSMANAGER_H
#define ACCESSMANAGER_H

#include <QHash>

class QNetworkAccessManager;
extern QNetworkAccessManager *access_manager;
extern QHash<QByteArray,QByteArray> referer_table;

#endif // NETWORKMANAGER_H
