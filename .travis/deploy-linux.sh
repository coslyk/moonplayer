#!/bin/sh

# Run make install
make install DESTDIR=.

# Create appimage
.travis/pkg2appimage .travis/appimage.yml