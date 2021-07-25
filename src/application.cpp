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

#include "application.h"
#include "playlistModel.h"
#include <QFileOpenEvent>
#include <QUrl>
#include <stdexcept>


Application::Application(int &argc, char **argv) : QGuiApplication(argc, argv)
{
    // Make file list
    for (int i = 1; i < argc; i++)
    {
        QByteArray f = argv[i];

        // Opened from browser extension
        if (f.startsWith("moonplayer://"))
        {
            f.replace("moonplayer://", "http://");
        }
        else if (f.startsWith("moonplayers://"))
        {
            f.replace("moonplayers://", "https://");
        }
        else if (f.startsWith("file://"))
        {
            f = QUrl::fromPercentEncoding(f.mid(7)).toUtf8();
        }

        m_fileList << f;
    }
}


bool Application::connectAnotherInstance()
{
    m_client = std::make_unique<QLocalSocket>();
    m_client->connectToServer(QStringLiteral("MoonPlayer_0817"), QLocalSocket::WriteOnly);
    m_client->waitForConnected();
    return m_client->state() == QLocalSocket::ConnectedState;
}


void Application::sendFileLists()
{
    Q_ASSERT(m_client != nullptr);

    if (m_fileList.isEmpty())
    {
        qDebug("Another instance is running.\n");
        return;
    }

    m_client->write(m_fileList.join('\n'));
    m_client->flush();
}


void Application::createServer()
{
    // If the previous instance crashes the pipe will remain in the filesystem
    QLocalServer::removeServer(QStringLiteral("MoonPlayer_0817"));

    // His birthday 1926.08.17
    // +1s
    m_server = std::make_unique<QLocalServer>();
    if (m_server->listen(QStringLiteral("MoonPlayer_0817")))
    {
        connect(m_server.get(), &QLocalServer::newConnection, this, &Application::onNewConnection);
    }
    else
    {
        throw std::runtime_error("Fails to create server.");
    }
    
}


void Application::processFileLists()
{
    processFileLists(m_fileList);
}


void Application::processFileLists(const QByteArrayList& fileList)
{
    Q_ASSERT(PlaylistModel::instance() != nullptr);

    if (!fileList.isEmpty())
    {
        QList<QUrl> localFileUrls;

        for (const auto& item : fileList)
        {
            if (item.startsWith("http://") || item.startsWith("https://"))
            {
                PlaylistModel::instance()->addUrl(QUrl::fromEncoded(item));
            }
            else
            {
                localFileUrls << QUrl::fromLocalFile(QString::fromLocal8Bit(item));
            }
        }

        if (!localFileUrls.isEmpty())
        {
            PlaylistModel::instance()->addLocalFiles(localFileUrls);
        }
    }
}


// File opened from another instance
void Application::onNewConnection()
{
    Q_ASSERT(m_server != nullptr);

    QLocalSocket *client = m_server->nextPendingConnection();

    connect(client, &QLocalSocket::readChannelFinished, [=]() {
        QByteArrayList fileList = client->readAll().split('\n');
        client->close();
        client->deleteLater();
        processFileLists(fileList);
    });
}

// File event for macOS
bool Application::event(QEvent *e)
{
    if (e->type() == QEvent::FileOpen)
    {
        Q_ASSERT(PlaylistModel::instance() != nullptr);
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(e);
        QString file = openEvent->file();
        if (file.isEmpty()) //url
        {
            QUrl url = openEvent->url();
            if (url.scheme() == QStringLiteral("moonplayer"))
                url.setScheme(QStringLiteral("http"));
            else if (url.scheme() == QStringLiteral("moonplayers"))
                url.setScheme(QStringLiteral("https"));
            PlaylistModel::instance()->addUrl(url);
        }
        else
        {
            PlaylistModel::instance()->addLocalFiles({ QUrl::fromLocalFile(file) });
        }
    }
    return QGuiApplication::event(e);
}