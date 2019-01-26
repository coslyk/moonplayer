#-------------------------------------------------
#
# Project created by QtCreator 2012-09-08T16:18:28
#
#-------------------------------------------------


QT += core gui network xml widgets websockets webenginewidgets
unix:!macx: QT += gui-private x11extras

macx:  TARGET = MoonPlayer
!macx: TARGET = moonplayer
TEMPLATE = app


SOURCES += main.cpp\
    playlist.cpp \
    skin.cpp \
    httpget.cpp \
    downloader.cpp \
    pyapi.cpp \
    settingsdialog.cpp \
    reslibrary.cpp \
    resplugin.cpp \
    mybuttongroup.cpp \
    detailview.cpp \
    utils.cpp \
    mylistwidget.cpp \
    cutterbar.cpp \
    videocombiner.cpp \
    plugin.cpp \
    selectiondialog.cpp \
    danmakuloader.cpp \
    accessmanager.cpp \
    platforms.cpp \
    downloaderitem.cpp \
    playerview.cpp \
    playercore.cpp \
    danmakudelaygetter.cpp \
    terminal.cpp \
    ykdlbridge.cpp \
    parserbridge.cpp \
    detectopengl.cpp \
    streamget.cpp \
    aboutdialog.cpp \
    cookiejar.cpp \
    youtubedlbridge.cpp \
    chromiumdebugger.cpp \
    extractor.cpp \
    simuparser.cpp \
    simuparserbridge.cpp
!macx: SOURCES += localserver.cpp \
    localsocket.cpp


TRANSLATIONS += moonplayer_zh_CN.ts


HEADERS  +=\
    playlist.h \
    skin.h \
    httpget.h \
    downloader.h \
    pyapi.h \
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
    videocombiner.h \
    settings_danmaku.h \
    playercore.h \
    plugin.h \
    selectiondialog.h \
    danmakudelaygetter.h \
    danmakuloader.h \
    platforms.h \
    downloaderitem.h \
    playerview.h \
    terminal.h \
    ykdlbridge.h \
    parserbridge.h \
    detectopengl.h \
    streamget.h \
    aboutdialog.h \
    cookiejar.h \
    youtubedlbridge.h \
    chromiumdebugger.h \
    extractor.h \
    simuparser.h \
    simuparserbridge.h
!macx: HEADERS += localserver.h \
    localsocket.h


FORMS    += \
    playlist.ui \
    settingsdialog.ui \
    reslibrary.ui \
    detailview.ui \
    cutterbar.ui \
    selectiondialog.ui \
    playerview.ui \
    aboutdialog.ui


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
    usr_share.files += plugins unblockcn moonplayer_*.qm ykdl_patched.py upgrade-*.sh
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
    FFMPEG.files = /usr/local/opt/ffmpeg/bin/ffmpeg
    FFMPEG.path = Contents/MacOS
    RESFILES.files = upgrade-plugins.sh upgrade-ykdl.sh upgrade-youtubedl.sh upgrade-parsers.sh ykdl_patched.py plugins unblockcn
    RESFILES.path = Contents/Resources
    TRANS_FILES.files = moonplayer_zh_CN.qm qt_zh_CN.qm
    TRANS_FILES.path = Contents/Resources/translations
    QMAKE_BUNDLE_DATA += RESFILES FFMPEG TRANS_FILES
    QMAKE_INFO_PLIST = Info.plist
    ICON = moonplayer.icns
}

# Windows icon
win32: RC_FILE = icon.rc

# Libraries
unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += python2 mpv
    INCLUDEPATH += $$PREFIX/include/qtermwidget5
    LIBS += -lqtermwidget5
}

macx {
    INCLUDEPATH += /System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7 \
        /usr/local/include
    LIBS += -F /System/Library/Frameworks -framework CoreFoundation \
        -L/usr/lib -ldl \
        -L/System/Library/Frameworks/Python.framework/Versions/2.7/lib/python2.7/config -lpython2.7 \
        -L/usr/local/lib -lmpv
}

DISTFILES += \
    Version
