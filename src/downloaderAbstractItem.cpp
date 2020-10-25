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

#include "downloaderAbstractItem.h"
#include <QFileInfo>

DownloaderAbstractItem::DownloaderAbstractItem(const QString& filepath, const QUrl &danmakuUrl, QObject* parent) :
    QObject(parent),
    m_name(QFileInfo(filepath).fileName()),
    m_filePath(filepath),
    m_danmakuUrl(danmakuUrl),
    m_state(WAITING),
    m_progress(0)
{
}

DownloaderAbstractItem::~DownloaderAbstractItem()
{
}


void DownloaderAbstractItem::setName(const QString& name)
{
    if (m_name != name)
    {
        m_name = name;
        emit nameChanged();
    }
}

void DownloaderAbstractItem::setFilePath(const QString& filePath)
{
    if (m_filePath != filePath)
    {
        m_filePath = filePath;
        emit filePathChanged();
    }
}


void DownloaderAbstractItem::setProgress(int progress)
{
    if (m_progress != progress)
    {
        m_progress = progress;
        emit progressChanged();
    }
}

void DownloaderAbstractItem::setState(DownloaderAbstractItem::State state)
{
    if (m_state != state)
    {
        m_state = state;
        emit stateChanged();

        // Write danmaku url
        if (state == FINISHED && !m_danmakuUrl.isEmpty())
        {
            QFile f(m_filePath + QStringLiteral(".danmaku"));
            if (f.open(QFile::WriteOnly))
            {
                f.write(m_danmakuUrl.toString().toUtf8());
                f.close();
            }
        }
    }
}
