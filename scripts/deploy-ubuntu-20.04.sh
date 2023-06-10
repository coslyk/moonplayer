#!/bin/sh

# Run make install
make install DESTDIR=AppDir

# Create appimage
export VERSION=${TRAVIS_TAG#v}
export QML_SOURCES_PATHS=$PWD/src/qml

wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy*.AppImage
./linuxdeploy-x86_64.AppImage --plugin qt --plugin qtfix --appdir AppDir --executable /usr/bin/ffmpeg --output appimage
