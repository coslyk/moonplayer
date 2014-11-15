#include <QApplication>
#include "player.h"
#include <QTranslator>
#include "settingsdialog.h"
#include "settings_player.h"
#include "playlist.h"
#include <QDir>
#include <QIcon>
#include <QLocale>
#include <QDebug>
#include <QTextCodec>
#include <Python.h>
#include "pyapi.h"
#include <iostream>
#ifdef Q_OS_LINUX
#include <QDBusInterface>
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    //check whether another MoonPlayer instance is running
#ifdef Q_OS_LINUX
    std::cout << "Checking another instance..." << std::endl;

    QDBusInterface iface("com.moonsoft.MoonPlayer", "/");
    if (iface.isValid())
    {
        if (argc == 1)
        {
            qDebug("Another instance is running. Quit.\n");
            return EXIT_FAILURE;
        }
        QString f = QString::fromUtf8(argv[1]);
        if (f.startsWith("http://"))
            iface.call("addUrl", f);
        else if (f.endsWith(".m3u") || f.endsWith("m3u8") || f.endsWith(".xspf")) //playlist
            iface.call("addList", f);
        else
            iface.call("addFileAndPlay", f.section('/', -1), f);
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
#ifdef Q_OS_WIN
    Settings::path = QString(argv[0]).section('\\', 0, -2);
#endif
    std::cout << "Initialize settings..." << std::endl;
    initSettings();

    std::cout << "Initialize API for Python..." << std::endl;
    initAPI();

    //translate moonplayer
    std::cout << "Initialize language support..." << std::endl;
    QTranslator translator;
    QDir path(Settings::path);
    translator.load(path.filePath("moonplayer_" + QLocale::system().name()));
    a.installTranslator(&translator);

    Player w;
    w.show();

    if (argc > 1)
    {
        std::cout << "Loading file..." << std::endl;
        QTextCodec* codec = QTextCodec::codecForLocale();
        QString file = codec->toUnicode(argv[1]);
        if (file.startsWith("http://"))
            w.playlist->addUrl(file);
        else if (file.endsWith(".m3u") || file.endsWith("m3u8") || file.endsWith(".xspf")) //playlist
            w.playlist->addList(file);
        else
            w.playlist->addFileAndPlay(file.section('/', -1), file);
    }
    a.exec();
    Py_Finalize();
    return 0;
}
