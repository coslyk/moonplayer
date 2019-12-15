#include "application.h"
#include "../playlistModel.h"
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
            PlaylistModel::instance()->addUrl(url);
        }
        else
        {
            QList<QUrl> files;
            files << QUrl::fromLocalFile(file);
            PlaylistModel::instance()->addLocalFiles(files);
        }
    }
    return QApplication::event(e);
}

void Application::onNewConnection()
{
}

