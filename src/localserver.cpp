#include "localserver.h"
#include "playlist.h"
#include <QLocalSocket>
#include <QTextCodec>

LocalServer::LocalServer(QObject *parent) : QLocalServer(parent)
{
    client = NULL;

    // His birthday 1926.08.17
    // +1s

    if (listen("MoonPlayer_0817"))
        connect(this, &LocalServer::newConnection, this, &LocalServer::onNewConnection);
    else
        qDebug("Fails to create server.");
}

LocalServer::~LocalServer()
{
    close();
}

void LocalServer::onNewConnection()
{
    client = nextPendingConnection();
    connect(client, &QLocalSocket::readChannelFinished, this, &LocalServer::readData);
}

void LocalServer::readData()
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
    client = NULL;
}

