#ifndef PARSER_H
#define PARSER_H

#include <QHash>
#include <QObject>
#include <QStringList>


namespace Parser {
//Read .xspf playlists
void readXspf(const QByteArray& xmlpage, QStringList& result);
}
#endif // PARSER_H
