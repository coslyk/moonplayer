#include "application.h"
#include "platform/localserver_no_mac.h"
#include "platform/localsocket_no_mac.h"
#include "playlist.h"
#include <QDir>
#include <QTextCodec>

Application::Application(int &argc, char **argv) :
    QApplication(argc, argv)
{
    this->argc = argc;
    this->argv = argv;
}

bool Application::parseArgs()
{
    QDir currentDir = QDir::current();

    // Check whether another MoonPlayer instance is running
    LocalSocket socket;
    if (socket.state() == LocalSocket::ConnectedState) // Is already running?
    {
        if (argc == 1)
        {
            qDebug("Another instance is running. Quit.\n");
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
                f = f.mid(7);

            // Online resource
            if (f.startsWith("http://") || f.startsWith("https://"))
                socket.addUrl(f);

            else // Local videos
            {
                // Convert to absolute path
                if (!QDir::isAbsolutePath(f))
                    f = currentDir.filePath(QTextCodec::codecForLocale()->toUnicode(f)).toLocal8Bit();
                // Playlist
                if (f.endsWith(".m3u") || f.endsWith("m3u8") || f.endsWith(".xspf"))
                    socket.addList(f);
                // First video
                else if (i == 1)
                    socket.addFileAndPlay(f);
                // Not first video
                else
                    socket.addFile(f);
            }
        }
        return false;
    }

    // This is the first instance, create server
     server = new LocalServer(this);

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
            file = file.mid(7);

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
