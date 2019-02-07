#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QStringList>
#include <QUrl>
#include <QVariant>


QString secToTime(int second, bool use_format = false);

//Read .xspf playlists
void readXspf(const QByteArray& xmlpage, QStringList& result);

//Save cookies to disk
bool saveCookies(const QUrl &url, const QString &filename);

#endif // MP_UTILS_H
