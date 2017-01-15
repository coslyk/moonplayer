#include "localsocket.h"

LocalSocket::LocalSocket(QObject *parent) : QLocalSocket(parent)
{
    connectToServer("MoonPlayer_0817", QLocalSocket::WriteOnly);
    waitForConnected();
}

LocalSocket::~LocalSocket()
{
    disconnectFromServer();
    if (state() == ConnectedState)
        waitForDisconnected();
}

void LocalSocket::addUrl(const QByteArray &url)
{
    write("addUrl " + url + '\n');
    flush();
}

void LocalSocket::addList(const QByteArray &list)
{
    write("addList " + list + '\n');
    flush();
}

void LocalSocket::addFileAndPlay(const QByteArray &file)
{
    write("addFileAndPlay " + file + '\n');
    flush();
}

void LocalSocket::addFile(const QByteArray &file)
{
    write("addFile " + file + '\n');
    flush();
}
