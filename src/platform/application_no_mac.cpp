#include "application.h"
#include "playlist.h"
#include <QDir>
#include <QLocalServer>
#include <QLocalSocket>
#include <QSurfaceFormat>
#include <QTextCodec>
#include <QUrl>

Application::Application(int &argc, char **argv) :
    QApplication(argc, argv)
{
    this->argc = argc;
    this->argv = argv;
    server = nullptr;
    client = nullptr;

    // force 32-bit color surface
    if (platformName().contains("wayland")) {
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
    if (server)
    {
        server->close();
        server = nullptr;
    }
}


bool Application::parseArgs()
{
    QDir currentDir = QDir::current();

    // Check whether another MoonPlayer instance is running
    QLocalSocket socket;
    socket.connectToServer("MoonPlayer_0817", QLocalSocket::WriteOnly);
    socket.waitForConnected();

    if (socket.state() == QLocalSocket::ConnectedState) // Is already running?
    {
        if (argc == 1)
        {
            qDebug("Another instance is running. Quit.\n");
            socket.disconnectFromServer();
            if (socket.state() == QLocalSocket::ConnectedState)
                socket.waitForDisconnected();
            return false;
        }

        // Read arguments
        for (int i = 1; i < argc; i++)
        {
            QByteArray f = argv[i];

            // Opened from browser extension
            if (f.startsWith("moonplayer://"))
                f.replace("moonplayer://", "http://");
            else if (f.startsWith("moonplayers://"))
                f.replace("moonplayers://", "https://");
            else if (f.startsWith("file://"))
                f = QUrl::fromPercentEncoding(f.mid(7)).toUtf8();

            // Online resource
            if (f.startsWith("http://") || f.startsWith("https://"))
                socket.write("addUrl " + f + '\n');

            else // Local videos
            {
                // Convert to absolute path
                if (!QDir::isAbsolutePath(f))
                    f = currentDir.filePath(QTextCodec::codecForLocale()->toUnicode(f)).toLocal8Bit();

                // Playlist
                if (f.endsWith(".m3u") || f.endsWith("m3u8") || f.endsWith(".xspf"))
                    socket.write("addList " + f + '\n');
                // First video
                else if (i == 1)
                    socket.write("addFileAndPlay " + f + '\n');
                // Not first video
                else
                    socket.write("addFile " + f + '\n');
            }
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
    QLocalServer::removeServer("MoonPlayer_0817");
    server = new QLocalServer(this);
    if (server->listen("MoonPlayer_0817"))
        connect(server, &QLocalServer::newConnection, this, &Application::onNewConnection);
    else
        qDebug("Fails to create server.");

    // Open files after player is loaded
    QMetaObject::invokeMethod(this, "openFiles", Qt::QueuedConnection);
    return true;
}

// Read arguments and open files
void Application::openFiles()
{
    QDir currentDir = QDir::current();

    // open file / url
    for (int i = 1; i < argc; i++)
    {
        QTextCodec* codec = QTextCodec::codecForLocale();
        QString file = codec->toUnicode(argv[i]);

        if (file.startsWith("--"))
            continue;

        // Opened from browser extension
        if (file.startsWith("moonplayer://"))
            file.replace("moonplayer://", "http://");
        else if (file.startsWith("moonplayers://"))
            file.replace("moonplayers://", "https://");
        else if (file.startsWith("file://"))
            file = QUrl::fromPercentEncoding(file.mid(7).toUtf8());

        // Online videos
        if (file.startsWith("http://") || file.startsWith("https://"))
            playlist->addUrl(file);

        // Local videos
        else
        {
            // Convert to absolute path
            if (!QDir::isAbsolutePath(file))
                file = currentDir.filePath(file);
            // Playlist
            if (file.endsWith(".m3u") || file.endsWith("m3u8") || file.endsWith(".xspf"))
                playlist->addList(file);
            // First video
            else if (i == 1)
                playlist->addFileAndPlay(file.section('/', -1), file);
            // Not first video
            else
                playlist->addFile(file.section('/', -1), file);
        }
    }
}


// File opened from another instance
void Application::onNewConnection()
{
    client = server->nextPendingConnection();
    connect(client, &QLocalSocket::readChannelFinished, this, &Application::readData);
}

void Application::readData()
{
    QTextCodec *codec = QTextCodec::codecForLocale();

    while (client->canReadLine())
    {
        QString data = codec->toUnicode(client->readLine());
        data.chop(1); // Remove '\n'

        if (data.startsWith("addUrl "))
            playlist->addUrl(data.mid(7));

        else if (data.startsWith("addList "))
            playlist->addList(data.mid(8));

        else if (data.startsWith("addFileAndPlay "))
            playlist->addFileAndPlay(data.section('/', -1), data.section(' ', 1));

        else if (data.startsWith("addFile "))
            playlist->addFile(data.section('/', -1), data.section(' ', 1));
    }

    client->close();
    client->deleteLater();
    client = nullptr;
}

