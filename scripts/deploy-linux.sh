#!/bin/sh

# Run make install
make install DESTDIR=AppDir

# Create appimage
QT_BASE_DIR=/opt/qt621
export QTDIR=$QT_BASE_DIR
export PATH=$QT_BASE_DIR/bin:$QT_BASE_DIR/libexec:$PATH
export LD_LIBRARY_PATH=$QT_BASE_DIR/lib/x86_64-linux-gnu:$QT_BASE_DIR/lib:$LD_LIBRARY_PATH
export PKG_CONFIG_PATH=$QT_BASE_DIR/lib/pkgconfig:$PKG_CONFIG_PATH
export VERSION=${TRAVIS_TAG#v}
export QMAKE=/opt/qt621/bin/qmake
export QML_SOURCES_PATHS=$PWD/src/qml

wget https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
wget https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy*.AppImage
./linuxdeploy-x86_64.AppImage --plugin qt --plugin qtfix --appdir AppDir --executable /usr/bin/ffmpeg --output appimage