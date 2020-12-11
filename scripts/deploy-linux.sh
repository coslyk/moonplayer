#!/bin/sh

# Run make install
make install DESTDIR=appdir
cp /usr/bin/ffmpeg ./appdir/usr/bin/

# Create appimage
export VERSION=${TRAVIS_TAG#v}

glibc_ver=$(ldd --version | sed -n "1s/^.* \([0-9.]*\)$/\1/p")
if [ ${glibc_ver#*.} -ge 27 ]; then
    no_check_glibc="-unsupported-allow-new-glibc"
    copyright="/usr/share/doc/libc6/copyright"
    install -p -T -D "$copyright" "appdir$copyright"
fi

if [ ! -x linuxdeployqt.AppImage ]; then
    curl -Lo linuxdeployqt.AppImage "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
    chmod a+x linuxdeployqt.AppImage
fi

wl_plugins="\
platforms/libqwayland-generic.so,\
platforms/libqwayland-egl.so,\
wayland-shell-integration/libxdg-shell.so,\
wayland-shell-integration/libxdg-shell-v6.so,\
wayland-decoration-client/libbradient.so,\
wayland-graphics-integration-client/libqt-plugin-wayland-egl.so"

exec ./linuxdeployqt.AppImage appdir/usr/share/applications/*.desktop $no_check_glibc -appimage -qmldir=src/qml/ -executable=appdir/usr/bin/ffmpeg -executable=appdir/usr/bin/moonplayer-hlsdl -exclude-libs=libdbus-1.so.3 -extra-plugins="$wl_plugins"
