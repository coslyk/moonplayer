#!/bin/sh

export PATH=/c/Qt/5.12.8/msvc2017_64/bin:$PATH

# Copy binary files
mkdir moonplayer
cp src/Release/moonplayer.exe src/scripts/update-parsers.ps1 libmpv/mpv-1.dll moonplayer/

# Bundle Qt
windeployqt moonplayer/moonplayer.exe --qmldir src/qml

# Bundle OpenSSL
curl -Lo openssl.zip https://indy.fulgan.com/SSL/Archive/openssl-1.0.0r-x64_86-win64.zip
unzip -o openssl.zip -d moonplayer

# Bundle ffmpeg
curl -Lo ffmpeg.7z https://www.gyan.dev/ffmpeg/builds/packages/ffmpeg-4.3.1-2020-10-01-essentials_build.7z
7z e ffmpeg.7z -omoonplayer ffmpeg-4.3.1-2020-10-01-essentials_build/bin/ffmpeg.exe

# Bundle hlsdl
curl -Lo hlsdl.7z https://rwijnsma.home.xs4all.nl/files/hlsdl/hlsdl-0.26-2bc52ab-win32-static-xpmod-sse.7z
7z e hlsdl.7z -omoonplayer hlsdl.exe

# Create installer
iscc scripts/win_installer.iss
mv scripts/Output/mysetup.exe ./MoonPlayer_${TRAVIS_TAG#v}_win_x64.exe
