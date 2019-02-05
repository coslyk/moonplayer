#include <QApplication>
#include <QTranslator>
#include "settingsdialog.h"
#include "accessmanager.h"
#include "application.h"
#include <locale.h>
#include <QDebug>
#include <QDir>
#include <QSettings>
#include "pyapi.h"
#include "platform/detectopengl.h"
#include "platform/paths.h"
#include "playerview.h"
#include "parserykdl.h"
#include "parseryoutubedl.h"
#include "parserwebcatch.h"
#ifdef Q_OS_WIN
#include <windows.h>
#endif


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
#include "platform/localserver_no_mac.h"
#include "platform/localsocket_no_mac.h"
#endif


int main(int argc, char *argv[])
{
    setenv("QTWEBENGINE_REMOTE_DEBUGGING", "19260", 1);

#if defined(Q_OS_LINUX)
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
    detectOpenGL();

#ifdef Q_OS_MAC
    MyApplication a(argc, argv);
#else
    Application a(argc, argv);
    if (!a.parseArgs())
        return 0;
#endif

#ifdef Q_OS_WIN
    // optimize font
    if (QLocale::system().country() == QLocale::China)
        QApplication::setFont(QFont("Microsoft Yahei", 9));
    // debug mode
    for (int i = 1; i < argc; i++)
    {
        if (QByteArray(argv[i]) == "--win-debug")
        {
            win_debug = AttachConsole(ATTACH_PARENT_PROCESS);
            if (win_debug)
            {
                freopen("CON", "w", stdout);
                freopen("CON", "w", stderr);
                freopen("CON", "r", stdin);
            }
            break;
        }
    }
#endif

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
