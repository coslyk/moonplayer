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

# Sources codes
SOURCES += \
    aboutdialog.cpp \
    accessmanager.cpp \
    chromiumdebugger.cpp \
    cutterbar.cpp \
    danmakudelaygetter.cpp \
    danmakuloader.cpp \
    detailview.cpp \
    downloader.cpp \
    downloaderitem.cpp \
    extractor.cpp \
    httpget.cpp \
    main.cpp \
    mybuttongroup.cpp \
    mylistwidget.cpp \
    parserbase.cpp \
    parserwebcatch.cpp \
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
    utils.cpp \
    videocombiner.cpp \
    platform/paths.cpp

HEADERS  +=\
    aboutdialog.h \
    accessmanager.h \
    chromiumdebugger.h \
    cutterbar.h \
    danmakudelaygetter.h \
    danmakuloader.h \
    detailview.h \
    downloader.h \
    downloaderitem.h \
    extractor.h \
    httpget.h \
    mybuttongroup.h \
    mylistwidget.h \
    parserbase.h \
    parserwebcatch.h \
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
    settings_video.h \
    settingsdialog.h \
    skin.h \
    streamget.h \
    utils.h \
    videocombiner.h \
    platform/application.h \
    platform/detectopengl.h \
    platform/paths.h \
    platform/terminal.h


# Platform specific source codes
unix:!macx {
    SOURCES += \
        platform/application_no_mac.cpp \
        platform/detectopengl_linux.cpp \
        platform/paths_linux.cpp \
        platform/terminal_linux.cpp
}

macx: {
    SOURCES += \
        platform/application_mac.cpp \
        platform/detectopengl_mac.cpp \
        platform/paths_mac.cpp \
        platform/terminal_mac.cpp
}


# Translations
TRANSLATIONS += moonplayer_zh_CN.ts


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
    usr_share.files += plugins unblockcn scripts ykdl_patched.py
    usr_share.path = $$PREFIX/share/moonplayer
    #translations
    trans.files += moonplayer_*.qm
    trans.path += $$PREFIX/share/moonplayer/translations
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
    INSTALLS += usr_share trans icon execute menu appdata
}

# Build bundle for Mac OS X
macx {
    FFMPEG.files = /usr/local/opt/ffmpeg/bin/ffmpeg
    FFMPEG.path = Contents/MacOS
    RESFILES.files = ykdl_patched.py plugins unblockcn scripts
    RESFILES.path = Contents/Resources
    QT_TRANS.files = /usr/local/opt/qt/translations
    QT_TRANS.path = Contents/Resources
    TRANS_FILES.files = moonplayer_zh_CN.qm
    TRANS_FILES.path = Contents/Resources/translations
    QMAKE_BUNDLE_DATA += RESFILES FFMPEG QT_TRANS TRANS_FILES
    QMAKE_INFO_PLIST = Info.plist
    ICON = moonplayer.icns
}

# Windows icon
win32: RC_FILE = icon.rc

# Libraries
unix:!macx {
    CONFIG += link_pkgconfig
    PKGCONFIG += python3 mpv
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
