#include "accessmanager.h"

QNetworkAccessManager *access_manager = 0;
QHash<QString,QByteArray> referer_table;
QStringList unseekable_hosts;
