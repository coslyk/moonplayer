#!/bin/bash
# Installs Qt on Windows for Travis CI.
#
# Copyright (C) 2019 Peter Wu <peter@lekensteyn.nl>
# SPDX-License-Identifier: GPL-2.0-or-later

set -eu -o pipefail

if [ -e "$QT5_BASE_DIR/bin/moc.exe" ] || [ -e "$QT5_BASE_DIR/bin/moc" ]; then
    echo "Found an existing Qt installation at $QT5_BASE_DIR"
    exit
fi

echo "Downloading the installer..."
# https is of no use if it redirects to a http mirror...
if [ "$TRAVIS_OS_NAME" = "windows" ]; then
    QT_INSTALLER=~/qt_installer.exe
    curl -Lo $QT_INSTALLER "http://download.qt.io/official_releases/online_installers/qt-unified-windows-x86-online.exe"
else
    export QT_QPA_PLATFORM=minimal  # No X server for Qt installer
    QT_INSTALLER=~/qt_installer.run
    curl -Lo $QT_INSTALLER "http://download.qt.io/official_releases/online_installers/qt-unified-linux-x64-online.run"
    chmod +x $QT_INSTALLER
fi

echo "Installing..."
# Run installer and save the installer output. To avoid hitting the timeout,
# periodically print some progress. On error, show the full log and abort.
$QT_INSTALLER --verbose --script .travis/qt-installer.qs |
    tee ~/qt-installer-output.txt |
    .travis/report-progress.sh ||
    (cat ~/qt-installer-output.txt; exit 1)

printf 'Installation size: '
du -sm "$QT5_BASE_DIR" 2>&1 ||
    (cat ~/qt-installer-output.txt; exit 1)
