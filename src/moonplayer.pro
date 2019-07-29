#-------------------------------------------------
#
# Project created by QtCreator 2012-09-08T16:18:28
#
#-------------------------------------------------

### Config
# QtWebEngine (Chromium) intergration, set ENABLE_WEBENGINE=no to disable it
!defined(ENABLE_WEBENGINE,var) {
    qtHaveModule(webenginewidgets): ENABLE_WEBENGINE=yes
    !qtHaveModule(webenginewidgets): ENABLE_WEBENGINE=no
}

# Library paths on Windows
win32 {
    !defined(PYTHON2_PATH,var) { PYTHON2_PATH=C:\\Python27 }
    !defined(LIBMPV_PATH,var) { LIBMPV_PATH=D:\\Develop\\mpv-dev }
}


QT += core gui network xml widgets
unix:!macx: QT += gui-private x11extras

macx:  TARGET = MoonPlayer
!macx: TARGET = moonplayer
TEMPLATE = app

# Sources codes
SOURCES += \
    aboutdialog.cpp \
    accessmanager.cpp \
    cutterbar.cpp \
    danmakudelaygetter.cpp \
    danmakuloader.cpp \
    detailview.cpp \
    downloader.cpp \
    downloaderitem.cpp \
    httpget.cpp \
    main.cpp \
    mybuttongroup.cpp \
    mylistwidget.cpp \
    parserbase.cpp \
    parserykdl.cpp \
    parseryoutubedl.cpp \
    playercore.cpp \
    playerview.cpp \
    playlist.cpp \
    pyapi.cpp \
    python_wrapper.cpp \
    reslibrary.cpp \
    resplugin.cpp \
    selectiondialog.cpp \
    settingsdialog.cpp \
    skin.cpp \
    streamget.cpp \
    upgraderdialog.cpp \
    utils.cpp \
    videocombiner.cpp \
    platform/paths.cpp

HEADERS  +=\
    aboutdialog.h \
    accessmanager.h \
    cutterbar.h \
    danmakudelaygetter.h \
    danmakuloader.h \
    detailview.h \
    downloader.h \
    downloaderitem.h \
    httpget.h \
    mybuttongroup.h \
    mylistwidget.h \
    parserbase.h \
    parserykdl.h \
    parseryoutubedl.h \
    playercore.h \
    playerview.h \
    playlist.h \
    pyapi.h \
    python_wrapper.h \
    reslibrary.h \
    resplugin.h \
    selectiondialog.h \
    settings_audio.h \
    settings_danmaku.h \
    settings_network.h \
    settings_player.h \
    settings_video.h \
    settingsdialog.h \
    skin.h \
    streamget.h \
    upgraderdialog.h \
    utils.h \
    videocombiner.h \
    platform/application.h \
    platform/detectopengl.h \
    platform/paths.h


# Platform specific source codes
unix:!macx {
    SOURCES += \
        platform/application_no_mac.cpp \
        platform/detectopengl_linux.cpp \
        platform/paths_linux.cpp
}

macx {
    SOURCES += \
        platform/application_mac.cpp \
        platform/detectopengl_mac.cpp \
        platform/paths_mac.cpp
}

win32 {
    SOURCES += \
        platform/application_no_mac.cpp \
        platform/detectopengl_win.cpp \
        platform/paths_win.cpp
}

# QtWebEngine Support (Mainly used for parsing youku videos)
equals(ENABLE_WEBENGINE, "yes") {
    QT += websockets webenginewidgets
    HEADERS += browser.h \
               chromiumdebugger.h \
               extractor.h \
               parserwebcatch.h
    SOURCES += browser.cpp \
               chromiumdebugger.cpp \
               extractor.cpp \
               parserwebcatch.cpp
    FORMS   += browser.ui
    DEFINES += MP_ENABLE_WEBENGINE
}


# Translations
TRANSLATIONS += translations/moonplayer_zh_CN.ts


FORMS    += \
    playlist.ui \
    settingsdialog.ui \
    reslibrary.ui \
    detailview.ui \
    cutterbar.ui \
    selectiondialog.ui \
    playerview.ui \
    aboutdialog.ui \
    upgraderdialog.ui


RESOURCES += \
    icons.qrc

# Installation on Linux
unix:!macx {
    isEmpty(PREFIX) {
        PREFIX = /usr/local
    }
    isEmpty(BINDIR) {
        BINDIR = bin
    }
    usr_share.files += plugins unblockcn translations
    usr_share.path = $$PREFIX/share/moonplayer
    #icon
    icon.files += icons/*
    icon.path = $$PREFIX/share/icons
    #appdata
    appdata.files += com.github.coslyk.MoonPlayer.appdata.xml
    appdata.path = $$PREFIX/share/metainfo
    #bin
    execute.files += moonplayer
    execute.path = $$PREFIX/$$BINDIR/
    #menu
    menu.files += com.github.coslyk.MoonPlayer.desktop
    menu.path = $$PREFIX/share/applications
    INSTALLS += usr_share icon execute menu appdata
}

# Build bundle for Mac OS X
macx {
    FFMPEG.files = /usr/local/bin/ffmpeg
    FFMPEG.path = Contents/MacOS
    RESFILES.files = plugins unblockcn translations
    RESFILES.path = Contents/Resources
    QT_TRANS.files = /usr/local/opt/qt/translations
    QT_TRANS.path = Contents/Resources/qt
    QMAKE_BUNDLE_DATA += RESFILES FFMPEG QT_TRANS
    QMAKE_INFO_PLIST = Info.plist
    ICON = moonplayer.icns
}

# Windows icon
win32: RC_FILE = icon.rc

# Libraries
unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += python3 mpv
}

macx {
    INCLUDEPATH += /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 \
        /usr/local/include
    LIBS += -F /System/Library/Frameworks -framework CoreFoundation \
        -L/usr/lib -ldl \
        -L/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config -lpython2.7 \
        -L/usr/local/lib -lmpv
}

win32 {
    INCLUDEPATH += \
        $$PYTHON2_PATH\\include \
        $$LIBMPV_PATH\\include
    LIBS += \
        $$PYTHON2_PATH\\libs\\python27.lib \
        $$LIBMPV_PATH\\i686\\mpv.lib
    QMAKE_CFLAGS  -= -Zc:strictStrings
    QMAKE_CXXFLAGS  -= -Zc:strictStrings
    QMAKE_CFLAGS_RELEASE  -= -Zc:strictStrings
    QMAKE_CFLAGS_RELEASE_WITH_DEBUGINFO -= -Zc:strictStrings
    QMAKE_CXXFLAGS_RELEASE  -= -Zc:strictStrings
    QMAKE_CXXFLAGS_RELEASE_WITH_DEBUGINFO -= -Zc:strictStrings
}

DISTFILES += \
    Version
