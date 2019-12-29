#!/bin/sh

export PATH=/c/Qt/5.12.3/msvc2017_64/bin:$PATH

mkdir moonplayer
cp src/Release/moonplayer.exe src/scripts/update-parsers.ps1 libmpv/mpv-1.dll moonplayer/
windeployqt moonplayer/moonplayer.exe --qmldir src/qml
pyinstaller -F src/scripts/hls_downloader.py --distpath moonplayer
pyinstaller -F src/scripts/danmaku2ass.py --distpath moonplayer
curl -Lo openssl.zip https://indy.fulgan.com/SSL/openssl-1.0.2u-x64_86-win64.zip
unzip -o openssl.zip -d moonplayer
iscc .travis/win_installer.iss
mv .travis/Output/mysetup.exe ./MoonPlayer_${TRAVIS_TAG}_win_x64.exe
