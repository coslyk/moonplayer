#include "application.h"
#include "../playlistModel.h"
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLocalServer>
#include <QLocalSocket>
#include <QTextCodec>
#include <QTimer>
#include <QUrl>


Application::Application(int &argc, char **argv) :
    QApplication(argc, argv)
{
    m_argc = argc;
    m_argv = argv;
    m_server = nullptr;
    m_client = nullptr;
}

Application::~Application()
{
    if (m_server)
    {
        m_server->close();
        m_server = NULL;
    }
}


bool Application::parseArgs()
{
    QStringList files;
    bool isLocalFiles = true;
    
    // Make file list
    for (int i = 1; i < m_argc; i++)
    {
        QByteArray f = m_argv[i];
        
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
        
        files << QTextCodec::codecForLocale()->toUnicode(f);
        
        // Online resource
        if (f.startsWith("http://") || f.startsWith("https://"))
        {
            isLocalFiles = false;
        }
    }
    
    // Check whether another MoonPlayer instance is running
    QLocalSocket socket;
    socket.connectToServer(QStringLiteral("MoonPlayer_0817"), QLocalSocket::WriteOnly);
    socket.waitForConnected();
    
    // Is already running?
    if (socket.state() == QLocalSocket::ConnectedState)
    {
        if (m_argc == 1)
        {
            qDebug("Another instance is running. Quit.\n");
            socket.disconnectFromServer();
            if (socket.state() == QLocalSocket::ConnectedState)
                socket.waitForDisconnected();
            return false;
        }
        
        else if (isLocalFiles)
        {
            QVariantHash msg;
            msg[QStringLiteral("action")] = QStringLiteral("addLocalFiles");
            msg[QStringLiteral("files")] = files;
            socket.write(QJsonDocument::fromVariant(msg).toJson());
        }
        
        else
        {
            QVariantHash msg;
            msg[QStringLiteral("action")] = QStringLiteral("addUrl");
            msg[QStringLiteral("url")] = files[0];
            socket.write(QJsonDocument::fromVariant(msg).toJson());
        }
        socket.flush();
        socket.disconnectFromServer();
        if (socket.state() == QLocalSocket::ConnectedState)
        {
            socket.waitForDisconnected();
        }
        return false;
    }

    // This is the first instance, create server
    // His birthday 1926.08.17
    // +1s
    QLocalServer::removeServer(QStringLiteral("MoonPlayer_0817"));
    m_server = new QLocalServer(this);
    if (m_server->listen(QStringLiteral("MoonPlayer_0817")))
    {
        connect(m_server, &QLocalServer::newConnection, this, &Application::onNewConnection);
    }
    else
    {
        qDebug("Fails to create server.");
    }

    // Open files after player is loaded
    if (m_argc > 1)
    {
        // Wait 0.5s to ensure that OpenGL is loaded
        QTimer::singleShot(500, [=]() {
            if (isLocalFiles)
            {
                QList<QUrl> fileUrls;
                for (const auto& file : files)
                {
                    fileUrls << QUrl::fromLocalFile(file);
                }
                PlaylistModel::instance()->addLocalFiles(fileUrls);
            }
            else
            {
                PlaylistModel::instance()->addUrl(QUrl(files[0]));
            }
        });
    }
    
    return true;
}


// File opened from another instance
void Application::onNewConnection()
{
    m_client = m_server->nextPendingConnection();
    connect(m_client, &QLocalSocket::readChannelFinished, [=]() {
        QJsonObject msg = QJsonDocument::fromJson(m_client->readAll()).object();
        m_client->close();
        m_client->deleteLater();
        m_client = nullptr;
        if (msg[QStringLiteral("action")] == QStringLiteral("addLocalFiles"))
        {
            QJsonArray files = msg[QStringLiteral("files")].toArray();
            QList<QUrl> fileUrls;
            for (const auto& file : files)
            {
                fileUrls << QUrl::fromLocalFile(file.toString());
            }
            PlaylistModel::instance()->addLocalFiles(fileUrls);
        }
        else if (msg[QStringLiteral("action")] == QStringLiteral("addUrl"))
        {
            PlaylistModel::instance()->addUrl(msg[QStringLiteral("url")].toString());
        }
    });
}
