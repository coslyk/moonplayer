#-------------------------------------------------
#
# Project created by QtCreator 2012-09-08T16:18:28
#
#-------------------------------------------------

QT             += core gui network xml widgets
unix:!macx: QT += dbus

macx:  TARGET = MoonPlayer
!macx: TARGET = moonplayer
TEMPLATE = app


SOURCES += main.cpp\
    player.cpp \
    playlist.cpp \
    webvideo.cpp \
    skin.cpp \
    httpget.cpp \
    downloader.cpp \
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
    classicplayer.cpp \
    plugin.cpp
!macx: SOURCES += playercore.cpp
macx: SOURCES += playercore_vlc.cpp
unix: SOURCES += danmakuloader.cpp \
    danmakudelaygetter.cpp \
    yougetbridge.cpp


TRANSLATIONS += moonplayer_zh_CN.ts


HEADERS  += player.h\
    playlist.h \
    webvideo.h \
    skin.h \
    httpget.h \
    downloader.h \
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
    classicplayer.h \
    playercore.h \
    plugin.h
unix: HEADERS += danmakuloader.h \
    danmakudelaygetter.h \
    yougetbridge.h


FORMS    += \
    player.ui \
    playlist.ui \
    transformer.ui \
    sortingdialog.ui \
    settingsdialog.ui \
    reslibrary.ui \
    detailview.ui \
    cutterbar.ui \
    classicplayer.ui


unix:!macx {
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

macx {
    VLCFILES.files = /Applications/VLC.app/Contents/MacOS/lib /Applications/VLC.app/Contents/MacOS/plugins
    VLCFILES.path = Contents/MacOS
    RESFILES.files = moonplayer_zh_CN.qm skins plugins
    RESFILES.path = Contents/Resources
    QMAKE_BUNDLE_DATA += RESFILES VLCFILES
    QMAKE_INFO_PLIST = Info.plist
    ICON = moonplayer.icns
}

win32: RC_FILE = icon.rc

unix:!macx: CONFIG += link_pkgconfig
unix:!macx: PKGCONFIG += python2

macx: INCLUDEPATH += /System/Library/Frameworks/Python.framework/Versions/2.7/Headers \
    /Applications/VLC.app/Contents/MacOS/include
macx: LIBS += -L/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config \
    -lpython2.7 -ldl -framework CoreFoundation \
    -L/Applications/VLC.app/Contents/MacOS/lib -lvlc -lvlccore

win32: INCLUDEPATH += C:\\Python27\\include
win32: LIBS += C:\\Python27\\libs\\python27.lib
