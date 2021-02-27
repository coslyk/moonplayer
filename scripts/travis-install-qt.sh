#!/bin/sh

if [ -e "$QT5_BASE_DIR/bin/moc.exe" ]; then
    echo "Found an existing Qt installation at $QT5_BASE_DIR"
    exit
fi

DESTDIR="C:\\Qt"
Base_URL="https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_51210/qt.qt5.51210.win64_msvc2017_64/5.12.10-0-202011040843qtbase-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
Declarative_URL="https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_51210/qt.qt5.51210.win64_msvc2017_64/5.12.10-0-202011040843qtdeclarative-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
QuickControls2_URL="https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_51210/qt.qt5.51210.win64_msvc2017_64/5.12.10-0-202011040843qtquickcontrols2-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"
Tools_URL="https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/qt5_51210/qt.qt5.51210.win64_msvc2017_64/5.12.10-0-202011040843qttools-Windows-Windows_10-MSVC2017-Windows-Windows_10-X86_64.7z"

# Download
curl -Lo base.7z "$Base_URL"
curl -Lo declarative.7z "$Declarative_URL"
curl -Lo quickcontrols2.7z "$QuickControls2_URL"
curl -Lo tools.7z "$Tools_URL"

# Extract
[ -d $DESTDIR ] || mkdir $DESTDIR
7z x -o$DESTDIR base.7z
7z x -o$DESTDIR declarative.7z
7z x -o$DESTDIR quickcontrols2.7z
7z x -o$DESTDIR tools.7z

QT_VERSION=`ls /c/Qt`
printf "[Paths]\nPrefix = .." > /c/Qt/$QT_VERSION/msvc2017_64/bin/qt.conf