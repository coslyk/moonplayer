#-------------------------------------------------
#
# Project created by QtCreator 2012-09-08T16:18:28
#
#-------------------------------------------------

QT       += core gui network xml
unix: QT += dbus


TARGET = moonplayer
TEMPLATE = app


SOURCES += main.cpp\
        player.cpp \
    mplayer.cpp \
    playlist.cpp \
    webvideo.cpp \
    skin.cpp \
    settings.cpp \
    parser.cpp \
    httpget.cpp \
    downloader.cpp \
    plugins.cpp \
    pyapi.cpp \
    transformer.cpp


TRANSLATIONS += moonplayer_zh_CN.ts


HEADERS  += player.h \
    mplayer.h \
    playlist.h \
    webvideo.h \
    skin.h \
    settings.h \
    parser.h \
    httpget.h \
    downloader.h \
    plugins.h \
    pyapi.h \
    transformer.h


FORMS    += \
    player.ui \
    playlist.ui \
    settings.ui \
    transformer.ui


unix {
    #skin
    default_skin.files += skins/default/*.png
    default_skin.path = /usr/share/moonplayer/skins/default
    #translation
    trans.files += moonplayer_*.qm
    trans.path = /usr/share/moonplayer
    #icon
    icon.files += moonplayer.png
    icon.path = /usr/share/icons
    #bin
    execute.files += moonplayer
    execute.path = /usr/bin
    #plugins
    plugin.files += plugins/*.py plugins/*.pyc
    plugin.path = /usr/share/moonplayer/plugins
    #menu
    menu.files += moonplayer.desktop
    menu.path = /usr/share/applications
    INSTALLS += default_skin execute trans icon menu plugin
}

RC_FILE = icon.rc

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += python2

win32: INCLUDEPATH += C:\\Python27\\include
win32: LIBS += C:\\Python27\\libs\\python27.lib
