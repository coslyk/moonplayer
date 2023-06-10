#!/bin/sh

# Copy binary files
mkdir moonplayer
cp src/Release/moonplayer.exe src/scripts/update-parsers.ps1 libmpv/mpv-2.dll moonplayer/

# Bundle Qt
windeployqt moonplayer/moonplayer.exe --qmldir src/qml

# Bundle OpenSSL
curl -Lo openssl.7z https://download.qt.io/online/qtsdkrepository/windows_x86/desktop/tools_openssl_x64/qt.tools.openssl.win_x64/1.1.1-17openssl_1.1.1q_prebuild_x64.7z
7z e openssl.7z -omoonplayer Tools/OpenSSL/Win_x64/bin/*.dll

# Bundle ffmpeg
curl -Lo ffmpeg.7z https://www.gyan.dev/ffmpeg/builds/packages/ffmpeg-5.1.2-essentials_build.7z
7z e ffmpeg.7z -omoonplayer ffmpeg-5.1.2-essentials_build/bin/ffmpeg.exe

# Bundle hlsdl
curl -Lo hlsdl.7z https://rwijnsma.home.xs4all.nl/files/hlsdl/hlsdl-0.27-883acbd-win32-static-xpmod-sse.7z
7z e hlsdl.7z -omoonplayer hlsdl.exe

# Create installer
iscc scripts/win_installer.iss
mv scripts/Output/mysetup.exe ./MoonPlayer_win_x64.exe
