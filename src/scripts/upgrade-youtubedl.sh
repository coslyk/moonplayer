#!/bin/sh

# Set OS-dependent variables
OS_NAME=`uname -s`
if [ "$OS_NAME" = 'Darwin' ]; then    # macOS
    VERSION_FILE="$HOME/Library/Application Support/MoonPlayer/youtube_dl/version.py"
    DEST_DIR="$HOME/Library/Application Support/MoonPlayer"
elif [ "$OS_NAME" = 'Linux' ]; then   # Linux
    XDG_DATA_HOME=${XDG_DATA_HOME:="$HOME/.local/share"}
    VERSION_FILE="$XDG_DATA_HOME/moonplayer/youtube_dl/version.py"
    DEST_DIR="$XDG_DATA_HOME/moonplayer"
    TMPDIR=${XDG_CACHE_HOME:="/tmp"}
else
    echo "Unsupported system!"
    exit 0
fi

# Set network tool
if which curl > /dev/null; then
    alias downloader="curl -L -o"
    alias fetcher="curl -s"
else
    alias downloader="wget -q -O"
    alias fetcher="wget -q -O -"
fi


cd "$TMPDIR"
echo ""
echo -e "\033[34m ---------- Checking youtube-dl's updates ----------- \033[0m"

# Get latest youtube-dl version
get_latest_version() {
    export PYTHONIOENCODING=utf8
    fetcher 'https://api.github.com/repos/rg3/youtube-dl/releases/latest' | \
        python -c "import sys, json; sys.stdout.write(json.load(sys.stdin)['tag_name'])"
}

LATEST_VERSION=`get_latest_version`
if [ -n "$LATEST_VERSION" ]; then
    echo "Latest version: $LATEST_VERSION"
else
    echo 'Error: Cannot get the latest version of youtube-dl. Please try again later.'
    exit 0
fi


# Get current youtube-dl version
if [ -e "$VERSION_FILE" ] && [ -d "$DEST_DIR" ]; then
    CURRENT_VERSION=`cat "$VERSION_FILE" | grep __version__ | sed "s/.*'\(.*\)'.*/\1/g"`
    echo "Current version: $CURRENT_VERSION"
    if [ "$CURRENT_VERSION" = "$LATEST_VERSION" ]; then
        echo "Youtube-dl already up-to-date."
        echo -e "\033[34m -------------------- End --------------------- \033[0m"
        exit 0
    fi
else
    echo "Current version: Not installed"
fi


# Download latest version
echo ""
echo -e "\033[34m ------------ Updating youtube-dl ------------- \033[0m"
echo "Downloading latest version..."
downloader ytdl.zip "https://api.github.com/repos/rg3/youtube-dl/zipball/$LATEST_VERSION"

echo ""
echo "Installing..."
rm -rf "$DEST_DIR/youtube_dl"
unzip -q ytdl.zip
cd rg3-youtube-dl-*
mv youtube_dl "$DEST_DIR/"
cd ..
rm -rf rg3-youtube-dl-*
rm -f ytdl.zip


echo ""
echo -e "\033[34m -------------------- End --------------------- \033[0m"


