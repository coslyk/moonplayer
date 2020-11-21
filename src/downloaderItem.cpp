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

#include "downloaderItem.h"
#include "fileDownloader.h"
#include <QDebug>
#include <QProcess>
#include <QSettings>
#include "dialogs.h"
#include "platform/paths.h"

QList <FileDownloader *> DownloaderItem::s_waiting;
std::atomic<int> DownloaderItem::s_threadCount(0);


DownloaderItem::DownloaderItem(const QString& filepath, const QList<QUrl>& urls, const QUrl& danmkauUrl, bool isDash, QObject* parent) :
    DownloaderAbstractItem(filepath, danmkauUrl, parent),
    m_process(nullptr),
    m_finished(0),
    m_total(urls.length()),
    m_isDash(isDash)
{
    // Create tempdir
    QString tempPath = QDir::tempPath() + QLatin1Char('/')  + name();
    m_tempDir = QDir(tempPath);
    if (!m_tempDir.exists())
    {
        m_tempDir.mkpath(tempPath);
    }
    
    // Start task
    QString fileSuffix = filepath.section(QLatin1Char('.'), -1);
    int maxThreads = QSettings().value(QStringLiteral("downloader/max_threads"), 5).toInt();

    for (int i = 0; i < m_total; i++)
    {
        QString itemFilePath;
        if (m_total == 1)  // Single item
        {
            itemFilePath = filepath;
        }
        else   // Multiple items, download them to temp first, then concat them later
        {
            itemFilePath = m_tempDir.filePath(QString::number(i).rightJustified(3, QLatin1Char('0'))) + QLatin1Char('.') + fileSuffix;
        }
        
        // Create downloader object
        FileDownloader* item = new FileDownloader(itemFilePath, urls[i], this);

        // Finish
        connect(item, &FileDownloader::finished, [this, item]() {
            // Remove it from the downloading list
            m_downloading.removeOne(item);
            item->deleteLater();

            // Continue waiting items
            s_threadCount--;
            continueWaitingItems();

            // Concat video clips
            if (++m_finished == m_total)
            {
                if (m_total > 1)
                {
                    concatVideos();
                }
                else
                {
                    setState(FINISHED);
                }   
            }
        });

        // Pause all items when one emits paused()
        connect(item, &FileDownloader::paused, [this]() {
            pause();
        });

        // Update download progress
        connect(item, &FileDownloader::progressChanged, [this](int) {
            int progress = m_finished * 100;
            for (const auto& item : m_downloading)
            {
                progress += item->progress();
            }
            progress /= m_total;
            setProgress(progress);
        });

        // Start downloads if thread count
        if (s_threadCount < maxThreads)
        {
            s_threadCount++;
            item->start();
            m_downloading << item;
        }
        else
        {
            s_waiting << item;
        }
    }
    setState(DOWNLOADING);
}

// Start
void DownloaderItem::start()
{
    if (state() != PAUSED)
        return;
    setState(DOWNLOADING);
    
    for (const auto& item : m_downloading)
    {
        item->start();
    }
}

// Pause
void DownloaderItem::pause()
{
    if (state() != DOWNLOADING)
        return;
    setState(PAUSED);

    for (const auto &item : m_downloading)
    {
        item->pause();
    }
}

// Stop
void DownloaderItem::stop()
{   
    if (state() != DOWNLOADING && state() != PAUSED)
        return;
    
    // Remove all downloading items
    int numDownloading = m_downloading.count();
    while (!m_downloading.isEmpty())
    {
        FileDownloader* item = m_downloading.takeFirst();
        item->disconnect();
        item->stop();
        item->deleteLater();
    }

    // Remove waiting items from waiting list
    auto waitings = children();
    for (const auto& waitingItem : waitings)
    {
        s_waiting.removeOne(reinterpret_cast<FileDownloader *>(waitingItem));
        waitingItem->disconnect();
        waitingItem->deleteLater();
    }

    setState(CANCELED);

    // Continue waiting tasks
    s_threadCount -= numDownloading;
    continueWaitingItems();
}


// Continue waiting tasks
void DownloaderItem::continueWaitingItems()
{
    int maxThreads = QSettings().value(QStringLiteral("downloader/max_threads"), 5).toInt();
    while (s_threadCount < maxThreads && !s_waiting.isEmpty())
    {
        s_threadCount++;
        FileDownloader* item = s_waiting.takeFirst();
        item->start();

        DownloaderItem* belonging = reinterpret_cast<DownloaderItem *>(item->parent());
        belonging->m_downloading << item;
    }
}


// Concat videos
void DownloaderItem::concatVideos()
{
    QStringList filelist = m_tempDir.entryList(QDir::Files, QDir::Name);
    QStringList args;
    
    // Youtube's dash videos?
    if (m_isDash)
    {
        args << QStringLiteral("-y") << QStringLiteral("-i") << filelist[0] << QStringLiteral("-i") << filelist[1];
        args << QStringLiteral("-c:v") << QStringLiteral("copy");
        if (filelist[0].endsWith(QStringLiteral(".mp4")))
            args << QStringLiteral("-c:a") << QStringLiteral("aac");
        else if (filelist[0].endsWith(QStringLiteral(".webm")))
            args << QStringLiteral("-c:a") << QStringLiteral("vorbis");
        args << QStringLiteral("-strict") << QStringLiteral("experimental") << filePath();
    }

    // Video clips
    else
    {
        // Write list file
        QFile file(m_tempDir.filePath(QStringLiteral("filelist.txt")));
        if (!file.open(QFile::WriteOnly))
        {
            setState(ERROR);
            Q_ASSERT(Dialogs::instance() != nullptr);
            Dialogs::instance()->messageDialog(tr("Error"), tr("Failed to write: ") + file.fileName());
            return;
        }
        for (const auto& filename : filelist)
        {
            file.write(QStringLiteral("file '%1'\n").arg(filename).toUtf8());
        }
        file.close();
    
        // Set mode to concat
        args << QStringLiteral("-y") << QStringLiteral("-f") << QStringLiteral("concat");

        // Set save level
        args << QStringLiteral("-safe") << QStringLiteral("0");

        // Set input
        args << QStringLiteral("-i") << QStringLiteral("filelist.txt");
        
        // Set output
        args << QStringLiteral("-c") << QStringLiteral("copy") << filePath();
    }

    // Run FFMPEG
    m_process = new QProcess(this);
    m_process->setWorkingDirectory(m_tempDir.absolutePath());
    connect(m_process, SIGNAL(finished(int)), this, SLOT(onConcatFinished(int)));
    connect(m_process, &QProcess::errorOccurred, [=]() { qDebug() << "FFMpeg ERROR: " << m_process->errorString(); });
    m_process->start(ffmpegFilePath(), args, QProcess::ReadOnly);
}

void DownloaderItem::onConcatFinished(int status)
{
    if (status == 0)  // Success, remove temp file
    {
        setState(FINISHED);
        QStringList filelist = m_tempDir.entryList(QDir::Files, QDir::Name);
        for (const auto& file : filelist)
        {
            m_tempDir.remove(file);
        }
        m_tempDir.cdUp();
        m_tempDir.rmdir(name());
    }
    else
    {
        setState(ERROR);
        Q_ASSERT(Dialogs::instance() != nullptr);
        Dialogs::instance()->messageDialog(tr("Error"), tr("Failed to concat: ") + filePath());
        qDebug("FFmpeg ERROR:\n%s", m_process->readAllStandardError().constData());
    }
    m_process->deleteLater();
    m_process = nullptr;
}

