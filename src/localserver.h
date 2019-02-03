#ifndef LOCALSERVER_H
#define LOCALSERVER_H

#include <QLocalServer>


class LocalServer : public QLocalServer
{
    Q_OBJECT
public:
    LocalServer(QObject *parent = nullptr);
    ~LocalServer();

private slots:
    void onNewConnection(void);
    void readData(void);

private:
    QLocalSocket *client;
};

#endif // LOCALSERVER_H
