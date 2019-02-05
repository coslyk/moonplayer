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

    // Translate moonplayer
    printf("Initialize language support...\n");
    QTranslator qtTranslator;
    if (qtTranslator.load("qt_" + QLocale::system().name(), getQtTranslationsPath()))
        a.installTranslator(&qtTranslator);

    QTranslator translator;
    if (translator.load("moonplayer_" + QLocale::system().name(), getAppPath() + "/translations"))
        a.installTranslator(&translator);

    // Create window
    PlayerView *player_view = new PlayerView;
    player_view->show();

    // Create video parsers
    parser_ykdl = new ParserYkdl(&a);
    parser_youtubedl = new ParserYoutubeDL(&a);
    parser_webcatch = new ParserWebCatch(&a);

    a.exec();
    Py_Finalize();
    delete player_view;
    return 0;
}
