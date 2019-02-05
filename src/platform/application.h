#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
class LocalServer;

/* MacOS handles file opening in a different way.
 * It don't use arguments, but FileOpenEvent.
 */

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    bool parseArgs(void);

#ifdef Q_OS_MAC
protected:
    bool event(QEvent *e);

#else
private slots:
    void openFiles(void);

private:
    int argc;
    char **argv;
    LocalServer *server;
#endif
};

#endif // APPLICATION_H
