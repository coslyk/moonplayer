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

#include "playlistModel.h"
#include <QFileInfo>
#include <QSettings>
#include "dialogs.h"
#include "mpvObject.h"
#include "parserGtv.h"
#include "parserYkdl.h"
#include "parserYoutubedl.h"

PlaylistModel* PlaylistModel::s_instance = nullptr;

PlaylistModel::PlaylistModel(QObject* parent) :
    QAbstractListModel(parent), m_playingIndex(-1)
{
    Q_ASSERT(s_instance == nullptr);
    s_instance = this;
}


// Add item to playlist
void PlaylistModel::addItem(const QString& title, const QUrl& fileUrl, const QUrl& danmakuUrl, const QUrl& audioTrackUrl)
{
    int index = m_titles.count();
    beginInsertRows(QModelIndex(), index, index);
    m_titles << title;
    m_fileUrls << fileUrl;
    m_danmakuUrls << danmakuUrl;
    m_audioTrackUrls << audioTrackUrl;
    endInsertRows();
    playItem(index);
}


void PlaylistModel::addItems(const QString& title, const QList<QUrl>& fileUrls, const QUrl& danmakuUrl, bool isDash)
{
    int start = m_titles.count();
    
    if (isDash)  // Youtube's dash videos
    {
        beginInsertRows(QModelIndex(), start, start);
        m_titles << title;
        m_fileUrls << fileUrls[0];   // First url is the video stream
        m_danmakuUrls << danmakuUrl;
        m_audioTrackUrls << fileUrls[1];  // Second url is the audio stream
        endInsertRows();
    }
    else    // Normal videos
    {
        int count = fileUrls.count();
        beginInsertRows(QModelIndex(), start, start + count - 1);
        for (int i = 0; i < count; i++)
        {
            m_titles << (title + QLatin1Char('_') + QString::number(i));
            m_fileUrls << fileUrls[i];
            m_danmakuUrls << (i == 0 ? danmakuUrl : QUrl());
            m_audioTrackUrls << QUrl();
        }
        endInsertRows();
    }
    playItem(start);
}

void PlaylistModel::addLocalFiles(const QList<QUrl>& fileUrls)
{
    int start = m_titles.count();
    int count = fileUrls.count();
    beginInsertRows(QModelIndex(), start, start + count - 1);
    for (int i = 0; i < count; i++)
    {
        m_titles << QFileInfo(fileUrls[i].toLocalFile()).fileName();
        m_fileUrls << fileUrls[i];
        m_audioTrackUrls << QUrl();
        QFile danmakuFile(fileUrls[i].toLocalFile() + QStringLiteral(".danmaku"));
        if (danmakuFile.open(QFile::ReadOnly | QFile::Text))
        {
            m_danmakuUrls << QString::fromUtf8(danmakuFile.readAll());
            danmakuFile.close();
        }
        else
        {
            m_danmakuUrls << QUrl();
        }
    }
    endInsertRows();
    playItem(start);
}



void PlaylistModel::addUrl ( const QUrl& url, bool download )
{
    Q_ASSERT(ParserYkdl::instance() != nullptr && ParserYoutubeDL::instance() != nullptr);

    if (ParserYkdl::isSupported(url))
        ParserYkdl::instance()->parse(url, download);
    else if (ParserGTV::isSupported(url))
        ParserGTV::instance()->parse(url, download);
    else
        ParserYoutubeDL::instance()->parse(url, download);
}


void PlaylistModel::addUrl(const QUrl& url)
{
    Q_ASSERT(Dialogs::instance() != nullptr);
    QSettings settings;
    OpenUrlAction action = static_cast<OpenUrlAction>(settings.value(QStringLiteral("player/url_open_mode")).toInt());
    if (action == QUESTION)
        Dialogs::instance()->openUrlDialog(url);
    else
        addUrl(url, action == DOWNLOAD);
}


void PlaylistModel::removeItem(int index)
{
    if (index < 0 || index >= m_titles.length())
        return;
    beginRemoveRows(QModelIndex(), index, index);
    m_titles.removeAt(index);
    m_fileUrls.removeAt(index);
    m_danmakuUrls.removeAt(index);
    m_audioTrackUrls.removeAt(index);
    endRemoveRows();
}

void PlaylistModel::clear()
{
    if (m_titles.count() == 0)
        return;
    beginRemoveRows(QModelIndex(), 0, m_titles.count() - 1);
    m_titles.clear();
    m_fileUrls.clear();
    m_danmakuUrls.clear();
    m_audioTrackUrls.clear();
    endRemoveRows();
}


void PlaylistModel::playItem(int index)
{
    Q_ASSERT(MpvObject::instance() != nullptr);

    if (index >= 0 && index < m_titles.count())
    {
        MpvObject::instance()->open(m_fileUrls[index], m_danmakuUrls[index], m_audioTrackUrls[index]);
    }
    if (m_playingIndex != index)
    {
        m_playingIndex = index;
        emit playingIndexChanged();
    }
}


void PlaylistModel::playNextItem()
{
    playItem(m_playingIndex + 1);
}



int PlaylistModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return m_titles.count();
}

QVariant PlaylistModel::data(const QModelIndex& index, int role) const
{
    int row = index.row();
    switch (role) {
        case TitleRole:
            return m_titles[row];
        default:
            return QVariant();
    }
}

QHash<int, QByteArray> PlaylistModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[TitleRole] = QByteArrayLiteral("title");
    return roles;
}


