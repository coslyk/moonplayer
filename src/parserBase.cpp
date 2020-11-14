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
#include <QPushButton>
#include <QRegularExpression>
#include <QUrl>
#include "accessManager.h"
#include "dialogs.h"
#include "downloader.h"
#include "playlistModel.h"

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
        Q_ASSERT(Dialogs::instance() != nullptr);
        Dialogs::instance()->selectionDialog(tr("Select streams"), result.stream_types, [=](int index) { finishStreamSelection(index); });
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
    QString msg = tr("Parse failed!\nURL:") + m_url.toString();
    msg += tr("Please try updating plugins.\n\nError output:\n") + errMsg;
    Dialogs::instance()->messageDialog(tr("Error"), msg);
}

