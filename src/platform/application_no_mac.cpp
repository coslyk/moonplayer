#include "application.h"
#include "../playlistModel.h"
#include <QDir>
#include <QJsonDocument>
#include <QLocalServer>
#include <QLocalSocket>
#include <QSurfaceFormat>
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

    // force 32-bit color surface
    if (platformName().contains(QStringLiteral("wayland"))) {
        QSurfaceFormat sf(QSurfaceFormat::defaultFormat());
        sf.setBlueBufferSize(8);
        sf.setGreenBufferSize(8);
        sf.setRedBufferSize(8);
        sf.setAlphaBufferSize(8);
        QSurfaceFormat::setDefaultFormat(sf);
    }
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
    QDir currentDir = QDir::current();
    m_isLocalFiles = true;
    
    // Make file list
    for (int i = 1; i < m_argc; i++)
    {
        QByteArray f = m_argv[i];
        
        // Opened from browser extension
        if (f.startsWith("moonplayer://"))
            f.replace("moonplayer://", "http://");
        
        else if (f.startsWith("moonplayers://"))
            f.replace("moonplayers://", "https://");
        
        else if (f.startsWith("file://"))
            f = QUrl::fromPercentEncoding(f.mid(7)).toUtf8();
        
        m_files << QTextCodec::codecForLocale()->toUnicode(f);
        
        // Online resource
        if (f.startsWith("http://") || f.startsWith("https://"))
            m_isLocalFiles = false;
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
        
        else if (m_isLocalFiles)
        {
            QVariantHash msg;
            msg[QStringLiteral("action")] = QStringLiteral("addLocalFiles");
            msg[QStringLiteral("files")] = m_files;
            socket.write(QJsonDocument::fromVariant(msg).toJson());
        }
        
        else
        {
            QVariantHash msg;
            msg[QStringLiteral("action")] = QStringLiteral("addUrl");
            msg[QStringLiteral("url")] = m_files[0];
            socket.write(QJsonDocument::fromVariant(msg).toJson());
        }
        socket.flush();
        socket.disconnectFromServer();
        if (socket.state() == QLocalSocket::ConnectedState)
            socket.waitForDisconnected();
        return false;
    }

    // This is the first instance, create server
    // His birthday 1926.08.17
    // +1s
    QLocalServer::removeServer(QStringLiteral("MoonPlayer_0817"));
    m_server = new QLocalServer(this);
    if (m_server->listen(QStringLiteral("MoonPlayer_0817")))
        connect(m_server, &QLocalServer::newConnection, this, &Application::onNewConnection);
    else
        qDebug("Fails to create server.");

    // Open files after player is loaded
    if (m_argc > 1)
    {
        // Wait 0.5s to ensure that OpenGL is loaded
        QTimer::singleShot(500, [=]() {
            if (m_isLocalFiles)
            {
                QList<QUrl> fileUrls;
                foreach (QString file, m_files)
                    fileUrls << QUrl::fromLocalFile(file);
                PlaylistModel::instance()->addLocalFiles(fileUrls);
            }
            else
            {
                PlaylistModel::instance()->addUrl(QUrl(m_files[0]));
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
        QVariantHash msg = QJsonDocument::fromJson(m_client->readAll()).toVariant().toHash();
        m_client->close();
        m_client->deleteLater();
        m_client = nullptr;
        if (msg[QStringLiteral("action")] == QStringLiteral("addLocalFiles"))
        {
            QStringList files = msg[QStringLiteral("files")].toStringList();
            QList<QUrl> fileUrls;
            foreach (QString file, files)
                fileUrls << QUrl::fromLocalFile(file);
            PlaylistModel::instance()->addLocalFiles(fileUrls);
        }
        else if (msg[QStringLiteral("action")] == QStringLiteral("addUrl"))
        {
            PlaylistModel::instance()->addUrl(msg[QStringLiteral("url")].toUrl());
        }
    });
}

bool Application::event(QEvent* e)
{
    return QApplication::event(e);
}
