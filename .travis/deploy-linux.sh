#!/bin/bash

# run make install
make install DESTDIR=.

# build appimages
.travis/pkg2appimage .travis/appimage-${UBUNTU_RELEASE}.yml

# build deb packages
if [ "$UBUNTU_RELEASE" = "bionic" ]; then
    checkinstall -y -D --install=no --fstrans=yes --pkgname=moonplayer --pkgversion=${TRAVIS_TAG#v} --pkgrelease=1~bionic --pkglicense=GPL_v3 --pkggroup=sound "--maintainer='coslyk <cos.lyk@gmail.com>'" --requires='ffmpeg, libmpv1, libqt5widgets5, libqt5network5, libqt5x11extras5, qml-module-qtquick2, qml-module-qtquick-controls2, qml-module-qtquick-dialogs, qml-module-qtquick-layouts, qml-module-qtquick-window2, qml-module-qt-labs-settings' --nodoc --exclude=/home
fi