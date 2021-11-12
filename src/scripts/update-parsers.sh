#!/bin/sh


# Copyright 2013-2020 Yikun Liu <cos.lyk@gmail.com>
#
# This program is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the
# License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
# Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with this program. If not, see http://www.gnu.org/licenses/.


# Set OS-dependent variables
OS_NAME=`uname -s`
if [ "$OS_NAME" = 'Darwin' ]; then    # macOS
    DEST_DIR="$HOME/Library/Application Support/MoonPlayer"
elif [ "$OS_NAME" = 'Linux' ]; then   # Linux
    XDG_DATA_HOME=${XDG_DATA_HOME:="$HOME/.local/share"}
    DEST_DIR="$XDG_DATA_HOME/moonplayer"
else
    echo "Unsupported system!"
    exit 0
fi

cd "$DEST_DIR"


# Set network tool
if which wget > /dev/null; then
    alias downloader="wget -q -O"
    alias fetcher="wget -q -O -"
else
    alias downloader="curl -s -L -o"
    alias fetcher="curl -s"
fi


# Set python
if which python3 > /dev/null; then
    PYTHON=python3
elif which python2 > /dev/null; then
    PYTHON=python2
else
    PYTHON=python
fi


# Define functions to check version
get_latest_version_github() {
    export PYTHONIOENCODING=utf8
    fetcher "https://api.github.com/repos/$1/releases/latest" | \
    $PYTHON -c "import sys, json; sys.stdout.write(json.load(sys.stdin)['tag_name'])"
}

get_current_version() {
    if [ -e "$DEST_DIR/version-$1.txt" ]; then
        cat "$DEST_DIR/version-$1.txt"
    fi
}

save_version_info() {
    echo "$2" > "version-$1.txt"
}


### Update yt-dlp
echo "\n-------- Checking yt-dlp's updates -------"

# Get latest yt-dlp version
CURRENT_VERSION=$(get_current_version "yt-dlp")
echo "Current version: $CURRENT_VERSION"

LATEST_VERSION=$(get_latest_version_github "yt-dlp/yt-dlp")
if [ -n "$LATEST_VERSION" ]; then
    echo "Latest version: $LATEST_VERSION"
else
    echo 'Error: Cannot get the latest version of yt-dlp. Please try again later.'
    exit 0
fi

if [ "$CURRENT_VERSION" = "$LATEST_VERSION" ]; then
    echo "Yt-dlp already up-to-date."
else
    # Download latest version
    echo "\n ------------ Updating yt-dlp -------------"
    echo "Downloading latest version..."
    rm -f yt-dlp
    downloader yt-dlp "https://github.com/yt-dlp/yt-dlp/releases/download/$LATEST_VERSION/yt-dlp"
    chmod a+x yt-dlp
    save_version_info "yt-dlp" "$LATEST_VERSION"
fi


### Update ykdl
echo "\n----------- Checking ykdl's updates ----------"

# Get current ykdl version
CURRENT_VERSION=$(get_current_version "ykdl")
echo "Current version: $CURRENT_VERSION"

# Get latest ykdl version
LATEST_VERSION=$(get_latest_version_github "coslyk/moonplayer-plugins")
if [ -n "$LATEST_VERSION" ]; then
    echo "Latest version: $LATEST_VERSION"
else
    echo 'Error: Cannot get the latest version of Ykdl. Please try again later.'
    exit 0
fi

if [ "$CURRENT_VERSION" = "$LATEST_VERSION" ]; then
    echo "Ykdl already up-to-date."
else
    # Download latest version
    echo "\n---------------- Updating ykdl ---------------"
    echo "Downloading latest version..."
    rm -f ykdl-moonplayer
    downloader ykdl-moonplayer "https://github.com/coslyk/moonplayer-plugins/releases/download/$LATEST_VERSION/ykdl-moonplayer"
    chmod a+x ykdl-moonplayer
    save_version_info "ykdl" "$LATEST_VERSION"
    
    echo "\n-------------- Updating plugins --------------"
    echo "Downloading latest version..."
    downloader plugins.zip "https://github.com/coslyk/moonplayer-plugins/releases/download/$LATEST_VERSION/plugins.zip"
    unzip -o plugins.zip -d plugins
    rm -f plugins.zip
    echo "Finished. You need to restart MoonPlayer to load plugins."
fi

