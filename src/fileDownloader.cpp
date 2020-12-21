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

#include "fileDownloader.h"
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>
#include "accessManager.h"


FileDownloader::FileDownloader(const QString& filepath, const QUrl& url, QObject* parent) :
    QObject(parent), m_file(filepath), m_url(url), m_lastPos(0)
{
    // Open file
    m_reply = nullptr;
    if (!m_file.open(QFile::WriteOnly))
    {
        qDebug() << (QStringLiteral("Create file failed: ") + filepath);
        return;
    }
}

FileDownloader::~FileDownloader()
{
    stop();
}


//start a request
void FileDownloader::start()
{
    if (m_reply != nullptr)
        return;
    
    // Continue from the last position
    QNetworkRequest request(m_url);
    if (m_lastPos)
    {
        request.setRawHeader(QByteArrayLiteral("Range"), QByteArrayLiteral("bytes=") + QByteArray::number(m_file.size()) + '-');
    }

    // Start download
    m_reply = NetworkAccessManager::instance()->get(request);
    connect(m_reply, &QNetworkReply::readyRead, this, &FileDownloader::onReadyRead);
    connect(m_reply, &QNetworkReply::finished, this, &FileDownloader::onFinished);
    connect(m_reply, &QNetworkReply::downloadProgress, this, &FileDownloader::onDownloadProgressChanged);

    emit started();
}


// Pause
void FileDownloader::pause()
{
    if (m_reply != nullptr)
    {
        m_reply->abort();
    }
}


// Stop
void FileDownloader::stop()
{
    if (m_reply == nullptr)
        return;
    
    m_reply->disconnect();
    m_reply->abort();
    m_reply->deleteLater();
    m_reply = nullptr;
    
    m_file.close();
    emit stopped();
}


void FileDownloader::onFinished()
{
    Q_ASSERT(m_reply);
    m_file.write(m_reply->readAll());

    // Redirect?
    int status = m_reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (status == 301 || status == 302) //redirect
    {
        m_reply->deleteLater();
        m_reply = nullptr;
        m_file.seek(0);
        m_lastPos = 0;
        m_url = QString::fromUtf8(m_reply->rawHeader(QByteArrayLiteral("Location")));
        start();
    }

    // Pause or error?
    else if (m_reply->error() != QNetworkReply::NoError)
    {
        QNetworkReply::NetworkError reason = m_reply->error();
        
        // Error?
        if (reason != QNetworkReply::OperationCanceledError)
        {
            qDebug() << (QStringLiteral("Http status code: %1\n%2\n").arg(QString::number(status), m_reply->errorString()));
        }
        
        m_lastPos = m_file.size();
        m_reply->deleteLater();
        m_reply = nullptr;
        emit paused();
    }

    // Finished
    else
    {
        m_reply->deleteLater();
        m_reply = nullptr;
        m_file.close();
        emit finished();
    }
}


void FileDownloader::onReadyRead()
{
    m_file.write(m_reply->readAll());
}

void FileDownloader::onDownloadProgressChanged(qint64 received, qint64 total)
{
    bool is_percentage = (total > 0);

    if (is_percentage)  // Total size is known
    {
        m_progress = (m_lastPos + received) * 100 / (m_lastPos + total);
    }
    else
    {
        m_progress = (m_lastPos + received) >> 20; //to MB
    }
    
    emit progressChanged(m_progress);
}
