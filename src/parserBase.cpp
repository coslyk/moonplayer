/* Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
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

#include "parserBase.h"
#include <QRegularExpression>
#include <QUrl>
#include "accessManager.h"
#include "dialogs.h"
#include "downloader.h"
#include "playlistModel.h"
#include "websiteSettings.h"

ParserBase::ParserBase(QObject *parent) : QObject(parent)
{
}


ParserBase::~ParserBase()
{
}


void ParserBase::parse(const QUrl &url, bool download)
{
    m_url = url;
    m_download = download;
    result.title.clear();
    result.stream_types.clear();
    result.streams.clear();
    result.danmaku_url.clear();
    runParser(url);
}


void ParserBase::finishParsing()
{
    // replace illegal chars in title with .
    static QRegularExpression illegalChars(QStringLiteral("[\\\\/]"));
    result.title.replace(illegalChars, QStringLiteral("."));
    
    // Stream is empty
    if (result.streams.isEmpty())
    {
        showErrorDialog(tr("The video has no streams. Maybe it is a VIP video and requires login."));
    }

    // Has only one stream, no selection needed
    else if (result.streams.count() == 1)
    {
        finishStreamSelection(0);
    }
    // More than one stream, selection is needed
    else
    {
        // Find stored profile first
        Q_ASSERT(WebsiteSettings::instance() != nullptr);
        QString storedProfile = WebsiteSettings::instance()->get(m_url.host());
        int index = result.stream_types.indexOf(storedProfile);
        if (index != -1)
        {
            finishStreamSelection(index);
            return;
        }
        // No stored profile found, show dialog
        Q_ASSERT(Dialogs::instance() != nullptr);
        Dialogs::instance()->selectionDialog(tr("Select streams"), result.stream_types, [this](int index, bool remember) {
            if (remember && index >= 0 && index < result.stream_types.size())
            {
                WebsiteSettings::instance()->set(m_url.host(), result.stream_types[index]);
            }
            finishStreamSelection(index);
        }, tr("Remember my choice"));
    }
}


void ParserBase::finishStreamSelection(int index)
{
    Q_ASSERT(NetworkAccessManager::instance() != nullptr);
    Q_ASSERT(Downloader::instance() != nullptr);
    Q_ASSERT(PlaylistModel::instance() != nullptr);

    Stream stream = result.streams[index];
    
    // Bind referer and user-agent
    if (!stream.referer.isEmpty())
    {
        for (const auto& url : stream.urls)
        {
            NetworkAccessManager::instance()->addReferer(url, stream.referer.toUtf8());
        }
    }
    if (!stream.ua.isEmpty())
    {
        for (const auto& url : stream.urls)
        {
            NetworkAccessManager::instance()->addUserAgent(url, stream.ua.toUtf8());
        }
    }
    if (!stream.seekable)
    {
        for (const auto& url : stream.urls)
        {
            NetworkAccessManager::instance()->addUnseekableHost(url.host());
        }
    }

    // Download
    if (m_download)
    {
        Downloader::instance()->addTasks(result.title + QLatin1Char('.') + stream.container, stream.urls, result.danmaku_url, stream.is_dash);
        emit downloadTasksAdded();
    }

    // Play
    else
    {
        PlaylistModel::instance()->addItems(result.title, stream.urls, result.danmaku_url, stream.is_dash);
    }
}


void ParserBase::showErrorDialog(const QString &errMsg)
{
    Q_ASSERT(Dialogs::instance() != nullptr);
    QString msg = tr("Parse failed!\nURL: %1\n\nPlease try updating plugins.").arg(m_url.toString());
    Dialogs::instance()->messageDialog(tr("Error"), msg);
    qDebug("%s", errMsg.toUtf8().constData());
}
