#!/bin/bash
# Installs Qt on Windows for Travis CI.
#
# Copyright (C) 2019 Peter Wu <peter@lekensteyn.nl>
# SPDX-License-Identifier: GPL-2.0-or-later

set -eu -o pipefail

if [ -e "$QT5_BASE_DIR/bin/moc.exe" ]; then
    echo "Found an existing Qt installation at $QT5_BASE_DIR"
    exit
fi

echo "Downloading the installer..."
# https is of no use if it redirects to a http mirror...
travis_wait curl -vLo qt_installer.exe "http://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-online.exe"

echo "Installing..."
travis_wait ./qt_installer.exe --verbose --script .travis/qt-installer-windows.qs
