#!/bin/sh

if [ -e "$QT5_BASE_DIR/bin/moc.exe" ] || [ -e "$QT5_BASE_DIR/bin/moc" ]; then
    echo "Found an existing Qt installation at $QT5_BASE_DIR"
    exit
fi

if [ "$TRAVIS_OS_NAME" = "windows" ]; then
    DESTDIR="C:\\Qt"
    Base_URL="http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5129/qt.qt5.5129.win64_msvc2017_64/5.12.9-0-202006121743qtbase-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
    Declarative_URL="http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5129/qt.qt5.5129.win64_msvc2017_64/5.12.9-0-202006121743qtdeclarative-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
    ImageFormats_URL="http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5129/qt.qt5.5129.win64_msvc2017_64/5.12.9-0-202006121743qtimageformats-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
    QuickControls2_URL="http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5129/qt.qt5.5129.win64_msvc2017_64/5.12.9-0-202006121743qtquickcontrols2-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
    Tools_URL="http://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_5129/qt.qt5.5129.win64_msvc2017_64/5.12.9-0-202006121743qttools-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
else
    DESTDIR="$HOME/Qt"
    Base_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5129/qt.qt5.5129.gcc_64/5.12.9-0-202006121744qtbase-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    Declarative_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5129/qt.qt5.5129.gcc_64/5.12.9-0-202006121744qtdeclarative-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    ImageFormats_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5129/qt.qt5.5129.gcc_64/5.12.9-0-202006121744qtimageformats-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    QuickControls2_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5129/qt.qt5.5129.gcc_64/5.12.9-0-202006121744qtquickcontrols2-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    Tools_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5129/qt.qt5.5129.gcc_64/5.12.9-0-202006121744qttools-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    X11Extras_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5129/qt.qt5.5129.gcc_64/5.12.9-0-202006121744qtx11extras-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    Wayland_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5129/qt.qt5.5129.gcc_64/5.12.9-0-202006121744qtwayland-Linux-RHEL_7_4-GCC-Linux-RHEL_7_4-X86_64.7z"
    ICU_URL="http://download.qt.io/online/qtsdkrepository/linux_x64/desktop/qt5_5129/qt.qt5.5129.gcc_64/5.12.9-0-202006121744icu-linux-Rhel7.2-x64.7z"
fi

# Download
curl -Lo base.7z "$Base_URL"
curl -Lo declarative.7z "$Declarative_URL"
curl -Lo imageformats.7z "$ImageFormats_URL"
curl -Lo quickcontrols2.7z "$QuickControls2_URL"
curl -Lo tools.7z "$Tools_URL"

# Extract
[ -d $DESTDIR ] || mkdir $DESTDIR
7z x -o$DESTDIR base.7z
7z x -o$DESTDIR declarative.7z
7z x -o$DESTDIR imageformats.7z
7z x -o$DESTDIR quickcontrols2.7z
7z x -o$DESTDIR tools.7z

# Linux specific
if [ "$TRAVIS_OS_NAME" = "linux" ]; then
    curl -Lo x11extras.7z "$X11Extras_URL"
    curl -Lo wayland.7z "$Wayland_URL"
    curl -Lo icu.7z "$ICU_URL"
    7z x -o$DESTDIR x11extras.7z
    7z x -o$DESTDIR wayland.7z
    7z x -o$DESTDIR icu.7z

    QT_VERSION=`ls $DESTDIR`
    printf "[Paths]\nPrefix = $DESTDIR/$QT_VERSION/gcc_64\n" > $DESTDIR/$QT_VERSION/gcc_64/bin/qt.conf

# Windows specific
elif [ "$TRAVIS_OS_NAME" = "windows" ]; then
    QT_VERSION=`ls /c/Qt`
    printf "[Paths]\nPrefix = ..\n" > /c/Qt/$QT_VERSION/msvc2017_64/bin/qt.conf
fi
