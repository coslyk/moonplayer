#-------------------------------------------------
#
# Project created by QtCreator 2012-09-08T16:18:28
#
#-------------------------------------------------

QT       += core gui network xml widgets
unix: QT += dbus


TARGET = moonplayer
TEMPLATE = app


SOURCES += main.cpp\
        player.cpp \
    mplayer.cpp \
    playlist.cpp \
    webvideo.cpp \
    skin.cpp \
    httpget.cpp \
    downloader.cpp \
    plugins.cpp \
    pyapi.cpp \
    transformer.cpp \
    sortingdialog.cpp \
    settingsdialog.cpp \
    reslibrary.cpp \
    resplugin.cpp \
    mybuttongroup.cpp \
    detailview.cpp \
    utils.cpp \
    mylistwidget.cpp \
    cutterbar.cpp \
    videocombiner.cpp \
    searcher.cpp \
    danmakudelaygetter.cpp
unix: SOURCES += danmakuloader.cpp


TRANSLATIONS += moonplayer_zh_CN.ts


HEADERS  += player.h \
    mplayer.h \
    playlist.h \
    webvideo.h \
    skin.h \
    httpget.h \
    downloader.h \
    plugins.h \
    pyapi.h \
    transformer.h \
    sortingdialog.h \
    settings_player.h \
    settings_video.h \
    settings_network.h \
    settingsdialog.h \
    reslibrary.h \
    resplugin.h \
    accessmanager.h \
    mybuttongroup.h \
    detailview.h \
    utils.h \
    mylistwidget.h \
    settings_audio.h \
    cutterbar.h \
    settings_plugins.h \
    videocombiner.h \
    searcher.h \
    settings_danmaku.h \
    danmakudelaygetter.h
unix: HEADERS += danmakuloader.h


FORMS    += \
    player.ui \
    playlist.ui \
    transformer.ui \
    sortingdialog.ui \
    settingsdialog.ui \
    reslibrary.ui \
    detailview.ui \
    cutterbar.ui


unix {
    #skin
    default_skin.files += skins
    default_skin.path = /usr/share/moonplayer
    #translation
    trans.files += moonplayer_*.qm
    trans.path = /usr/share/moonplayer
    #danmaku
    danmaku.files += danmaku2ass.py
    danmaku.path = /usr/share/moonplayer
    #icon
    icon.files += moonplayer.png
    icon.path = /usr/share/icons
    #bin
    execute.files += moonplayer
    execute.path = /usr/bin
    #plugins
    plugin.files += plugins
    plugin.path = /usr/share/moonplayer
    #menu
    menu.files += moonplayer.desktop
    menu.path = /usr/share/applications
    INSTALLS += default_skin execute trans icon menu plugin danmaku
}

RC_FILE = icon.rc

unix: CONFIG += link_pkgconfig
unix: PKGCONFIG += python2

win32: INCLUDEPATH += C:\\Python27\\include
win32: LIBS += C:\\Python27\\libs\\python27.lib
