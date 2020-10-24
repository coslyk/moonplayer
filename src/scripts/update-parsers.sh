#!/bin/sh

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
    alias downloader="curl -L -o"
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
get_latest_version() {
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


### Update youtube-dl
echo "\n-------- Checking youtube-dl's updates -------"

# Get latest youtube-dl version
cat << EOF
Temporarily the youtube-dl is not awailable due to DCMA takedown. See

    https://github.com/ytdl-org/youtube-dl/

for more information.

To use youtube-dl, you can download it manually and put it into system's
PATH, or install it with the package manager.

EOF


### Update ykdl
echo "\n----------- Checking ykdl's updates ----------"

# Get current ykdl version
CURRENT_VERSION=$(get_current_version "ykdl")
echo "Current version: $CURRENT_VERSION"

# Get latest ykdl version
LATEST_VERSION=$(get_latest_version "coslyk/moonplayer-plugins")
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

