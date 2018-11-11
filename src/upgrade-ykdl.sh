#!/bin/sh

# Set OS-dependent variables
OS_NAME=`uname -s`
if [ "$OS_NAME" = 'Darwin' ]; then    # macOS
    VERSION_FILE="$HOME/Library/Application Support/MoonPlayer/ykdl-version.txt"
    DEST_DIR="$HOME/Library/Application Support/MoonPlayer/ykdl"
elif [ "$OS_NAME" = 'Linux' ]; then   # Linux
    XDG_DATA_HOME=${XDG_DATA_HOME:="$HOME/.local/share"}
    VERSION_FILE="$XDG_DATA_HOME/moonplayer/ykdl-version.txt"
    DEST_DIR="$XDG_DATA_HOME/moonplayer/ykdl"
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
echo -e "\033[34m ---------- Checking ykdl's updates ----------- \033[0m"

# Get latest ykdl version
get_latest_version() {
    export PYTHONIOENCODING=utf8
    fetcher 'https://api.github.com/repos/zhangn1985/ykdl/branches/master' | \
        python -c "import sys, json; sys.stdout.write(json.load(sys.stdin)['commit']['sha'])"
}

LATEST_VERSION=`get_latest_version`
if [ -n "$LATEST_VERSION" ]; then
    echo "Latest version: $LATEST_VERSION"
else
    echo 'Error: Cannot get the latest version of ykdl. Please try again later.'
    exit 0
fi


# Get current ykdl version
if [ -e "$VERSION_FILE" ] && [ -d "$DEST_DIR" ]; then
    CURRENT_VERSION=`cat "$VERSION_FILE"`
    echo "Current version: $CURRENT_VERSION"
    if [ "$CURRENT_VERSION" = "$LATEST_VERSION" ]; then
        echo "Ykdl already up-to-date."
        echo -e "\033[34m -------------------- End --------------------- \033[0m"
        exit 0
    fi
else
    echo "Current version: Not installed"
fi


# Download latest version
echo ""
echo -e "\033[34m --------------- Updating ykdl ---------------- \033[0m"
echo "Downloading https://github.com/zhangn1985/ykdl/archive/master.zip"
downloader ykdl.zip "https://github.com/zhangn1985/ykdl/archive/master.zip"

echo ""
echo "Installing..."
unzip -q ykdl.zip
rm -rf "$DEST_DIR"
mv ykdl-master "$DEST_DIR"
rm -f ykdl.zip


# Save version info
echo "$LATEST_VERSION" > "$VERSION_FILE"

echo ""
echo -e "\033[34m -------------------- End --------------------- \033[0m"

