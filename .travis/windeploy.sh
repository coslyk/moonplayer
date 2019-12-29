#!/bin/sh

export PATH=/c/Qt/5.12.3/msvc2017_64/bin:$PATH

mkdir moonplayer
cp src/Release/moonplayer.exe src/scripts/update-parsers.ps1 libmpv/mpv-1.dll moonplayer/
windeployqt moonplayer/moonplayer.exe --qmldir src/qml
zip -9 -r MoonPlayer_${TRAVIS_TAG}_windows_x64.zip moonplayer
