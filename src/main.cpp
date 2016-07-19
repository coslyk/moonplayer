#include <QApplication>
#include <QTranslator>
#include "classicplayer.h"
#include "settingsdialog.h"
#include "settings_player.h"
#include "playlist.h"
#include "accessmanager.h"
#include <QDir>
#include <QIcon>
#include <QLocale>
#include <QDebug>
#include <QTextCodec>
#include <QNetworkAccessManager>
#include <Python.h>
#include "pyapi.h"
#include "player.h"
#ifdef Q_OS_LINUX
#include <QDBusInterface>
#endif

QNetworkAccessManager *access_manager = NULL;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QDir currentDir = QDir::current();

    //check whether another MoonPlayer instance is running
#ifdef Q_OS_LINUX
    printf("Checking another instance...\n");

    QDBusInterface iface("com.moonsoft.MoonPlayer", "/");
    if (iface.isValid())
    {
        if (argc == 1)
        {
            qDebug("Another instance is running. Quit.\n");
            return EXIT_FAILURE;
        }
        for (int i = 1; i < argc; i++)
        {
            QString f = QString::fromUtf8(argv[i]);
            //online resource
            if (f.startsWith("http://"))
                iface.call("addUrl", f);
            //playlist
            else if (f.endsWith(".m3u") || f.endsWith("m3u8") || f.endsWith(".xspf")) //playlist
                iface.call("addList", f);
            //local videos
            else
            {
                if (!f.contains('/'))    //not an absolute path
                    f = currentDir.filePath(f);
                if (i == 1)   //first video
                    iface.call("addFileAndPlay", f.section('/', -1), f);
                else
                    iface.call("addFile", f.section('/', -1), f);
            }
        }
        return EXIT_SUCCESS;
    }
    QDBusConnection conn = QDBusConnection::sessionBus();
    if (!conn.registerService("com.moonsoft.MoonPlayer"))
    {
        qDebug() << conn.lastError().message();
        return EXIT_FAILURE;
    }
#endif

    //init
    access_manager = new QNetworkAccessManager(&a);
    printf("Initialize settings...\n");
    initSettings();

    printf("Initialize API for Python...\n");
    initAPI();

    //translate moonplayer
    printf("Initialize language support...\n");
    QTranslator translator;
    QDir path(Settings::path);
    translator.load(path.filePath("moonplayer_" + QLocale::system().name()));
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
            if (!file.contains('/'))    //not an absolute path
                file = currentDir.filePath(file);
            if (i == 1) //first video
                playlist->addFileAndPlay(file.section('/', -1), file);
            else
                playlist->addFile(file.section('/', -1), file);
        }
    }
    a.exec();
    Py_Finalize();
    if (classic_player)
        delete classic_player;
    else
        delete player;
    return 0;
}
