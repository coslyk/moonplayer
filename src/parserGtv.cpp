/* Copyright 2013-2020 Aven <turineaven@github>
 *
 * This program is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see http://www.gnu.org/licenses/.
 */

#include "parserGtv.h"
#include "accessManager.h"
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QTextStream>
#include "dialogs.h"

ParserGTV ParserGTV::s_instance;

bool ParserGTV::isSupported(const QUrl& url)
{
    return (url.scheme() == QStringLiteral("http") || url.scheme() == QStringLiteral("https")) &&
        url.host() == QStringLiteral("gtv.org") &&
        url.path().startsWith(QStringLiteral("/video/id="));
}

void ParserGTV::runParser(const QUrl& url)
{
    bool ok;
    QString id = url.path().mid(10);
    long value = id.leftRef(8).toLong(&ok, 16);
    if (!ok)
    {
        showErrorDialog(QStringLiteral("Unrecognized gtv.org URL"));
        return;
    }
    result.title = id;

    QDateTime time = QDateTime::fromSecsSinceEpoch(value, Qt::UTC);
    QString m3u8 = QStringLiteral("https://filegroup.gtv.org/group5/vm3u8/") +
                   time.toString(QStringLiteral("yyyyMMdd/hh/mm/")) + id +
                   QStringLiteral("/hls.m3u8");

    QNetworkRequest request(m3u8);
    QNetworkReply *reply = NetworkAccessManager::instance()->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(replyFinished()));
}

void ParserGTV::replyFinished(void)
{
    QString error;
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(QObject::sender());
    if (!reply->error())
    {
        QTextStream stream(reply);
        QRegExp rx(QStringLiteral("RESOLUTION=([0-9x]{3,})"));
        QString base = reply->url().toString();
        base.truncate(base.lastIndexOf(QLatin1Char('/')) + 1);
        for (QString line; !(line = stream.readLine().trimmed()).isNull();)
        {
            if (line.startsWith(QStringLiteral("#EXT-X-STREAM-INF:")) &&
                rx.indexIn(line) >= 0)
            {
                result.stream_types << rx.cap(1);
            }
            else if (line.endsWith(QStringLiteral(".m3u8")) &&
                     result.stream_types.count() == result.streams.count() + 1)
            {
                Stream stream;
                stream.container = QStringLiteral("m3u8");
                stream.referer = QStringLiteral("https://gtv.org/");
                stream.seekable = true;
                stream.is_dash = false;
                stream.urls << base + line;
                result.streams << stream;
            }
        }

        if (result.streams.size() &&
            result.streams.size() == result.stream_types.count())
            finishParsing();
        else
            error = QStringLiteral("Unable to parse .m3u8 playlist");
    }
    else
    {
        error = reply->errorString();
    }
    reply->deleteLater();

    if (!error.isEmpty())
        showErrorDialog(error);
}
