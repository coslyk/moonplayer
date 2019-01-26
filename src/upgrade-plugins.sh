#!/bin/sh

# Set OS-dependent variables
OS_NAME=`uname -s`
if [ "$OS_NAME" = 'Darwin' ]; then    # macOS
    VERSION_FILE="$HOME/Library/Application Support/MoonPlayer/plugins-version.txt"
    DEST_DIR="$HOME/Library/Application Support/MoonPlayer/plugins/"
elif [ "$OS_NAME" = 'Linux' ]; then   # Linux
    XDG_DATA_HOME=${XDG_DATA_HOME:="$HOME/.local/share"}
    VERSION_FILE="$XDG_DATA_HOME/moonplayer/plugins-version.txt"
    DEST_DIR="$XDG_DATA_HOME/moonplayer/plugins/"
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

# Remove plugins using outdated APIs
cd "$DEST_DIR"
rm -f extractor_*


cd "$TMPDIR"
echo ""
echo -e "\033[34m --------- Checking plugins' updates ---------- \033[0m"

# Get latest plugin version
get_latest_version() {
    export PYTHONIOENCODING=utf8
    fetcher 'https://api.github.com/repos/coslyk/moonplayer-plugins/branches/master' | \
        python -c "import sys, json; sys.stdout.write(json.load(sys.stdin)['commit']['sha'])"
}

LATEST_VERSION=`get_latest_version`
if [ -n "$LATEST_VERSION" ]; then
    echo "Latest version: $LATEST_VERSION"
else
    echo 'Error: Cannot check the updates of plugins. Please try again later.'
    exit 0
fi


# Get current plugins version
if [ -e "$VERSION_FILE" ] && [ -d "$DEST_DIR" ]; then
    CURRENT_VERSION=`cat "$VERSION_FILE"`
    echo "Current version: $CURRENT_VERSION"
    if [ "$CURRENT_VERSION" = "$LATEST_VERSION" ]; then
        echo "Plugins already up-to-date."
        echo -e "\033[34m -------------------- End --------------------- \033[0m"
        exit 0
    fi
else
    echo "Current version: Not installed"
fi


# Download latest version
echo ""
echo -e "\033[34m -------------- Updating plugins --------------- \033[0m"
echo "Downloading https://github.com/coslyk/moonplayer-plugins/archive/master.zip"
downloader moonplayer_plugins.zip "https://github.com/coslyk/moonplayer-plugins/archive/master.zip"

echo ""
echo "Installing..."
unzip -q moonplayer_plugins.zip
cd moonplayer-plugins-master
mv -f *.py "$DEST_DIR"
cd ..
rm -f moonplayer_plugins.zip
rm -r moonplayer-plugins-master


# Save version info
echo "$LATEST_VERSION" > "$VERSION_FILE"
echo "Successfully upgraded plugins. Please relaunch MoonPlayer to apply changes."
echo ""
echo -e "\033[34m -------------------- End --------------------- \033[0m"

