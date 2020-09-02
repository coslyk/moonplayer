#!/bin/sh

# Use curl-slim to reduce AppImage size
sudo apt-get -y remove curl libcurl4-openssl-dev libcurl3-gnutls libcurl3-nss libcurl3
wget 'https://launchpad.net/~djcj/+archive/ubuntu/libcurl-slim/+files/curl_7.59.0-1~xenial2_amd64.deb'
wget 'https://launchpad.net/~djcj/+archive/ubuntu/libcurl-slim/+files/libcurl3_7.59.0-1~xenial2_amd64.deb'
sudo apt-get -y install ./*.deb

# Run make install
make install DESTDIR=appdir
cp /usr/bin/ffmpeg ./appdir/usr/bin/

# Create appimage
export VERSION=${TRAVIS_TAG#v}
curl -Lo linuxdeployqt.AppImage "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt.AppImage
./linuxdeployqt.AppImage appdir/usr/share/applications/*.desktop -appimage -qmldir=src/qml/ -exclude-libs=libdbus-1.so.3
