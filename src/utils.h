#ifndef UTILS_H
#define UTILS_H

#include <QHash>
#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>


QString secToTime(int second, bool use_format = false);

// Read .xspf playlists
void readXspf(const QByteArray& xmlpage, QStringList& result);

// Read / save QHash
void saveQHashToFile(const QHash<QString, QString> &hash, const QString &filename);
QHash<QString, QString> loadQHashFromFile(const QString &filename);

#endif // MP_UTILS_H
