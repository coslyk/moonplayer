#include <QApplication>
#include <QTranslator>
#include "classicplayer.h"
#include "settingsdialog.h"
#include "settings_player.h"
#include "playlist.h"
#include "accessmanager.h"
#include "updatechecker.h"
#include <locale.h>
#include <QDir>
#include <QIcon>
#include <QLocale>
#include <QDebug>
#include <QTextCodec>
#include <QNetworkAccessManager>
#include <Python.h>
#include "pyapi.h"
#include "player.h"

#ifdef Q_OS_MAC
#include <QFileOpenEvent>
class MyApplication : public QApplication
{
public:
    MyApplication(int &argc, char **argv)
        : QApplication(argc, argv)
    {
    }
    bool event(QEvent *event)
    {
        if (event->type() == QEvent::FileOpen) {
            QFileOpenEvent *openEvent = static_cast<QFileOpenEvent *>(event);
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
        return QApplication::event(event);
    }
};
#else
#include "localserver.h"
#include "localsocket.h"
#endif


int main(int argc, char *argv[])
{
    QDir currentDir = QDir::current();

#ifdef Q_OS_MAC
    MyApplication a(argc, argv);
#else
    QApplication a(argc, argv);

    //check whether another MoonPlayer instance is running
    LocalSocket socket;
    if (socket.state() == LocalSocket::ConnectedState) //Is already running
    {
        if (argc == 1)
        {
            qDebug("Another instance is running. Quit.\n");
            return EXIT_FAILURE;
        }

        for (int i = 1; i < argc; i++)
        {
            QByteArray f = argv[i];
            //online resource
            if (f.startsWith("http://"))
                socket.addUrl(f);

            //playlist
            else if (f.endsWith(".m3u") || f.endsWith("m3u8") || f.endsWith(".xspf"))
                socket.addList(f);

            //local videos
            else
            {
                if (!QDir::isAbsolutePath(f))
                    f = currentDir.filePath(QTextCodec::codecForLocale()->toUnicode(f)).toLocal8Bit();

                if (i == 1)   //first video
                    socket.addFileAndPlay(f);
                else
                    socket.addFile(f);
            }
        }
        return EXIT_SUCCESS;
    }

    // This is the first instance, create server
    LocalServer server;
#endif
    //for mpv
    setlocale(LC_NUMERIC, "C");

    //init
    access_manager = new QNetworkAccessManager(&a);
    printf("Initialize settings...\n");
    initSettings();

    printf("Initialize API for Python...\n");
    initAPI();

    //translate moonplayer
    printf("Initialize language support...\n");
    QDir path(Settings::path);
#ifdef Q_OS_MAC
    path.cd("translations");
    //translate application menu
    QTranslator qtTranslator;
    if (qtTranslator.load(path.filePath("qt_" + QLocale::system().name())))
        a.installTranslator(&qtTranslator);
#endif
    QTranslator translator;
    if (translator.load(path.filePath("moonplayer_" + QLocale::system().name())))
        a.installTranslator(&translator);

    ClassicPlayer *classic_player = NULL;
    Player *player = NULL;
    if (Settings::disableSkin)
    {
        classic_player = new ClassicPlayer;
        classic_player->show();
    }
    else
    {
        player = new Player;
        player->show();
    }

    for (int i = 1; i < argc; i++)
    {
        QTextCodec* codec = QTextCodec::codecForLocale();
        QString file = codec->toUnicode(argv[i]);
        if (file.startsWith("http://"))
            playlist->addUrl(file);
        else if (file.endsWith(".m3u") || file.endsWith("m3u8") || file.endsWith(".xspf")) //playlist
            playlist->addList(file);
        else
        {
            if (!QDir::isAbsolutePath(file))    //not an absolute path
                file = currentDir.filePath(file);
            if (i == 1) //first video
                playlist->addFileAndPlay(file.section('/', -1), file);
            else
                playlist->addFile(file.section('/', -1), file);
        }
    }

    // Check new version
    UpdateChecker checker;
    checker.check();

    a.exec();
    Py_Finalize();
    if (classic_player)
        delete classic_player;
    else
        delete player;
    return 0;
}
