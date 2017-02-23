#include "accessmanager.h"

QNetworkAccessManager *access_manager = 0;
QHash<QByteArray,QByteArray> referer_table;
