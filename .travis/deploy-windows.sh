#!/bin/sh

export PATH=/c/Qt/5.12.6/msvc2017_64/bin:$PATH

# Copy binary files
mkdir moonplayer
cp src/Release/moonplayer.exe src/scripts/update-parsers.ps1 libmpv/mpv-1.dll moonplayer/

# Bundle Qt
windeployqt moonplayer/moonplayer.exe --qmldir src/qml

# Bundle OpenSSL
curl -Lo openssl.zip https://indy.fulgan.com/SSL/openssl-1.0.2u-x64_86-win64.zip
unzip -o openssl.zip -d moonplayer

# Bundle ffmpeg
curl -Lo ffmpeg.zip https://ffmpeg.zeranoe.com/builds/win64/static/ffmpeg-4.2.1-win64-static.zip
unzip -j ffmpeg.zip ffmpeg-4.2.1-win64-static/bin/ffmpeg.exe -d moonplayer

# Create installer
iscc .travis/win_installer.iss
mv .travis/Output/mysetup.exe ./MoonPlayer_${TRAVIS_TAG#v}_win_x64.exe
