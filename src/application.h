#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>
class LocalServer;

class Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    bool parseArgs(void);

private slots:
    void openFiles(void);

private:
    int argc;
    char **argv;
    LocalServer *server;
};

#endif // APPLICATION_H
