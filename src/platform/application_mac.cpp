#include "application.h"
#include "playlist.h"
#include <QFileOpenEvent>

Application::Application(int &argc, char **argv) :
    QApplication(argc, argv)
{
}

Application::~Application()
{
}

// MacOS don't use arguments to open files. Always return true here
bool Application::parseArgs()
{
     return true;
}

bool Application::event(QEvent *e)
{
    if (e->type() == QEvent::FileOpen) {
        QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(e);
        QString file = openEvent->file();
        if (file.isEmpty()) //url
        {
            QUrl url = openEvent->url();
            if (url.scheme() == "moonplayer")
                url.setScheme("http");
            else if (url.scheme() == "moonplayers")
                url.setScheme("https");
            playlist->addUrl(url.toString());
        }
        else if (file.endsWith(".m3u") || file.endsWith(".m3u8") || file.endsWith(".xspf"))
            playlist->addList(file);
        else
            playlist->addFileAndPlay(file.section('/', -1), file);
    }
    return QApplication::event(e);
}
