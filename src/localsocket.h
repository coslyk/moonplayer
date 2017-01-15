#ifndef LOCALSOCKET_H
#define LOCALSOCKET_H

#include <QLocalSocket>


class LocalSocket : public QLocalSocket
{
    Q_OBJECT
public:
    LocalSocket(QObject *parent = NULL);
    ~LocalSocket();
    void addUrl(const QByteArray &url);
    void addList(const QByteArray &list);
    void addFileAndPlay(const QByteArray &file);
    void addFile(const QByteArray &file);
};

#endif // LOCALSOCKET_H
