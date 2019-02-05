#include <QApplication>
#include <QTranslator>
#include "settingsdialog.h"
#include "accessmanager.h"
#include <locale.h>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include "pyapi.h"
#include "platform/application.h"
#include "platform/detectopengl.h"
#include "platform/paths.h"
#include "playerview.h"
#include "parserykdl.h"
#include "parseryoutubedl.h"
#include "parserwebcatch.h"

int main(int argc, char *argv[])
{
    setenv("QTWEBENGINE_REMOTE_DEBUGGING", "19260", 1);
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    detectOpenGL();

    Application a(argc, argv);
    if (!a.parseArgs())
        return 0;

    //for mpv
    setlocale(LC_NUMERIC, "C");

    //init
    access_manager = new NetworkAccessManager(&a);
    printf("Initialize settings...\n");
    initSettings();

    printf("Initialize API for Python...\n");
    initPython();

    //translate moonplayer
    printf("Initialize language support...\n");
    QDir path(getAppPath());
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

    PlayerView *player_view = new PlayerView;
    player_view->show();

    // create video parsers
    parser_ykdl = new ParserYkdl(&a);
    parser_youtubedl = new ParserYoutubeDL(&a);
    parser_webcatch = new ParserWebCatch(&a);

    a.exec();
    Py_Finalize();
    delete player_view;
    return 0;
}
