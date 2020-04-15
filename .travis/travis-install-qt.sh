#!/bin/sh

if [ -e "$QT5_BASE_DIR/bin/moc.exe" ] || [ -e "$QT5_BASE_DIR/bin/moc" ]; then
    echo "Found an existing Qt installation at $QT5_BASE_DIR"
    exit
fi

if [ "$TRAVIS_OS_NAME" = "windows" ]; then
    DESTDIR="C:\\Qt"
    Base_URL="http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5128/qt.qt5.5128.win64_msvc2017_64/5.12.8-0-202004051457qtbase-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
    Declarative_URL="http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5128/qt.qt5.5128.win64_msvc2017_64/5.12.8-0-202004051457qtdeclarative-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
    ImageFormats_URL="http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5128/qt.qt5.5128.win64_msvc2017_64/5.12.8-0-202004051457qtimageformats-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
    QuickControls_URL="http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5128/qt.qt5.5128.win64_msvc2017_64/5.12.8-0-202004051457qtquickcontrols-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
    QuickControls2_URL="http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5128/qt.qt5.5128.win64_msvc2017_64/5.12.8-0-202004051457qtquickcontrols2-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
    Tools_URL="http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5128/qt.qt5.5128.win64_msvc2017_64/5.12.8-0-202004051457qttools-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
else
    DESTDIR="$HOME/Qt"
    Base_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5128/qt.qt5.5128.gcc_64/5.12.8-0-202004051456qtbase-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    Declarative_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5128/qt.qt5.5128.gcc_64/5.12.8-0-202004051456qtdeclarative-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    ImageFormats_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5128/qt.qt5.5128.gcc_64/5.12.8-0-202004051456qtimageformats-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    QuickControls_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5128/qt.qt5.5128.gcc_64/5.12.8-0-202004051456qtquickcontrols-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    QuickControls2_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5128/qt.qt5.5128.gcc_64/5.12.8-0-202004051456qtquickcontrols2-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    Tools_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5128/qt.qt5.5128.gcc_64/5.12.8-0-202004051456qttools-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    X11Extras_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5128/qt.qt5.5128.gcc_64/5.12.8-0-202004051456qtx11extras-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    ICU_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5128/qt.qt5.5128.gcc_64/5.12.8-0-202004051456icu-linux-Rhel7.2-x64.7z"
fi

# Download
curl -Lo base.7z "$Base_URL"
curl -Lo declarative.7z "$Declarative_URL"
curl -Lo imageformats.7z "$ImageFormats_URL"
curl -Lo quickcontrols.7z "$QuickControls_URL"
curl -Lo quickcontrols2.7z "$QuickControls2_URL"
curl -Lo tools.7z "$Tools_URL"

# Extract
[ -d $DESTDIR ] || mkdir $DESTDIR
7z x -o$DESTDIR base.7z
7z x -o$DESTDIR declarative.7z
7z x -o$DESTDIR imageformats.7z
7z x -o$DESTDIR quickcontrols.7z
7z x -o$DESTDIR quickcontrols2.7z
7z x -o$DESTDIR tools.7z

# Linux specific
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    curl -Lo x11extras.7z "$X11Extras_URL"
    curl -Lo icu.7z "$ICU_URL"
    7z x -o$DESTDIR x11extras.7z
    7z x -o$DESTDIR icu.7z

    QT_VERSION=`ls $DESTDIR`
    printf "[Paths]\nPrefix = $DESTDIR/$QT_VERSION/gcc_64" > $DESTDIR/$QT_VERSION/gcc_64/bin/qt.conf
fi