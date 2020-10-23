#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
class QLocalServer;
class QLocalSocket;

/* MacOS handles file opening in a different way.
 * It doesn't use arguments, but FileOpenEvent.
 */

class Application : public QApplication
{
public:
    Application(int &argc, char **argv);
    virtual ~Application();
    bool parseArgs(void);
    
#ifdef Q_OS_MAC
protected:
    bool event(QEvent *e);
    
#else
private:
    void onNewConnection(void);
    int m_argc;
    char **m_argv;
    QLocalServer *m_server;
    QLocalSocket *m_client;
#endif
};

#endif // APPLICATION_H
