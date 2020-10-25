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

#ifndef DOWNLOADERMULTIPLEITEM_H
#define DOWNLOADERMULTIPLEITEM_H

#include "downloaderAbstractItem.h"
#include <QDir>
#include <atomic>

class QProcess;
class FileDownloader;

class DownloaderItem : public DownloaderAbstractItem
{
    Q_OBJECT

public:
    DownloaderItem(const QString &filepath, const QList<QUrl>& urls, const QUrl& danmakuUrl = QUrl(), bool isDash = false, QObject *parent = nullptr);
    virtual void pause(void) override;
    virtual void start(void) override;
    virtual void stop(void) override;
    
private:
    static QList<FileDownloader*> s_waiting;
    static std::atomic<int> s_threadCount;
    static void continueWaitingItems(void);
    
    QList<FileDownloader*> m_downloading;
    QDir m_tempDir;
    QProcess* m_process;
    std::atomic<int> m_finished;
    int m_total;
    bool m_isDash;
    
    void concatVideos(void);
    
private slots:
    void onConcatFinished(int status);
};

#endif // DOWNLOADERMULTIPLEITEM_H
