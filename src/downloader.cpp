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

#include "downloader.h"
#include <QCoreApplication>
#include <QDir>
#include <QSettings>
#include "downloaderHlsItem.h"
#include "downloaderItem.h"

    Downloader::Downloader(QObject *parent) : QObject(parent)
{
}

Downloader::~Downloader()
{
}

Downloader * Downloader::instance()
{
    static Downloader* c_instance = nullptr;
    if (c_instance == nullptr)
        c_instance = new Downloader(qApp);
    return c_instance;
}


void Downloader::addTasks(const QString& filename, const QList<QUrl>& urls, const QUrl& danmakuUrl, bool isDash)
{
    QSettings settings;
    QDir dir(settings.value(QStringLiteral("downloader/save_to")).toUrl().toLocalFile());
    QString filepath = dir.filePath(filename);
    DownloaderAbstractItem* item;
    
    if (urls[0].path().endsWith(QStringLiteral(".m3u8")))
    {
        item = new DownloaderHlsItem(filepath, urls[0], danmakuUrl, this);
    }
    else
    {
        item = new DownloaderItem(filepath, urls, danmakuUrl, isDash, this);
    }
    m_model << item;
    emit modelUpdated();
}

QObjectList Downloader::model()
{
    return m_model;
}

