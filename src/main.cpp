#include <QApplication>
#include <QDir>
#include <QTranslator>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QSettings>
#include <clocale>
#include "accessManager.h"
#include "downloader.h"
#include "downloaderAbstractItem.h"
#include "mpvObject.h"
#include "parserYkdl.h"
#include "parserYoutubedl.h"
#include "playlistModel.h"
#include "platform/application.h"
#include "platform/detectOpengl.h"
#include "platform/paths.h"
#include "plugin.h"
#include "utils.h"

int main(int argc, char *argv[])
{
    qputenv("PYTHONIOENCODING", "utf-8");

    detectOpenGLEarly();
    
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    Application app(argc, argv);
    app.setOrganizationName("coslyk");
    app.setOrganizationDomain("coslyk.github.io");
    app.setApplicationName("MoonPlayer");
    app.setApplicationVersion(MOONPLAYER_VERSION);
    
    detectOpenGLLate();
    
    if (!app.parseArgs())
        return 0;
    
    
    // Qt sets the locale in the QGuiApplication constructor, but libmpv
    // requires the LC_NUMERIC category to be set to "C", so change it back.
    std::setlocale(LC_NUMERIC, "C");
    qputenv("LC_NUMERIC", "C");
    
    // Translate
    QTranslator translator;
    if (translator.load(QLocale::system().name(), ":/l10n"))
        app.installTranslator(&translator);

    qmlRegisterType<MpvObject>("MoonPlayer", 1, 0, "MpvObject");
    qmlRegisterSingletonType(QUrl("qrc:/qml/Color.qml"), "MoonPlayer", 1, 0, "Color");
    qmlRegisterUncreatableType<DownloaderAbstractItem>("MoonPlayer", 1, 0, "DownloaderItem", "Access to enums & flags only");
    
    QQmlApplicationEngine engine;

    // Set UI style
    if (QSettings().value("player/use_system_frame").toBool())
    {
        engine.addImportPath("qrc:/qml/classicUI");
        qputenv("QT_QUICK_CONTROLS_STYLE", "fusion");
    }
    else
    {
        engine.addImportPath("qrc:/qml/modernUI");
        qputenv("QT_QUICK_CONTROLS_MATERIAL_VARIANT", "Dense");
        qputenv("QT_QUICK_CONTROLS_STYLE", "material");
    }

    QQmlContext* context = engine.rootContext();
    Downloader* downloader = Downloader::instance();
    context->setContextProperty("accessManager", NetworkAccessManager::instance());
    context->setContextProperty("downloaderModel", QVariant::fromValue(downloader->model()));
    context->setContextProperty("playlistModel", PlaylistModel::instance());
    context->setContextProperty("plugins", QVariant::fromValue(Plugin::loadPlugins()));
    context->setContextProperty("ykdl", ParserYkdl::instance());
    context->setContextProperty("youtube_dl", ParserYoutubeDL::instance());
    context->setContextProperty("utils", new Utils());
    
    // Update downloader model
    QObject::connect(downloader, &Downloader::modelUpdated, [=](){
        context->setContextProperty("downloaderModel", QVariant::fromValue(downloader->model()));
    });
    
    engine.load(QUrl(QStringLiteral("qrc:/qml/main.qml")));
    
    // Check updates
    Utils::checkUpdate();
    
    // Create user resources dir
    if (!QDir(userResourcesPath()).exists())
        QDir().mkpath(userResourcesPath());
    
    return app.exec();
}
