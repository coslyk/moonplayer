#!/bin/bash


# Check whether Python3 is installed
which python3 > /dev/null || {
echo -e "\033[31m *********** Error *********** \033[0m"
echo -e "\033[31m Python3 is not installed. Please download it from \033[0m"
echo -e "\033[31m https://www.python.org/downloads/mac-osx/ \033[0m"
echo -e "\033[31m and then install it. \033[0m"
echo -e "\033[31m ************ End ************ \033[0m"
exit 0
}


cd $TMPDIR
echo ""
echo -e "\033[34m ---------- Checking updates --------- \033[0m"

# Read version from version.py
get_version() {
    while read LINE; do
        if [ "${LINE%% *}" = '__version__' ]; then  # Like __version__ = '0.4.626'
            VERSION=${LINE##* }
            VERSION=${VERSION//"'"/}
            echo "$VERSION"
            break
        fi
    done
}

# Get latest you-get version
VERSION_URL='https://raw.githubusercontent.com/soimort/you-get/develop/src/you_get/version.py'
LATEST_VERSION=`curl -s $VERSION_URL | get_version`
if [ -n "$LATEST_VERSION" ]; then
    echo "Latest version: $LATEST_VERSION"
else
    echo 'Error: Cannot get the latest version of you-get. Please try again later.'
    exit 0
fi

# Get current you-get version
VERSION_FILE="$HOME/Library/Application Support/MoonPlayer/you-get/src/you_get/version.py"
if [ -e "$VERSION_FILE" ]; then
    CURRENT_VERSION=`cat "$VERSION_FILE" | get_version`
    echo "Current version: $CURRENT_VERSION"
    if [ "$CURRENT_VERSION" = "$LATEST_VERSION" ]; then
        echo "You-get already up-to-date."
        echo -e "\033[34m ---------------- End ---------------- \033[0m"
        exit 0
    fi
else
    echo "Current version: Not installed"
fi

# Download latest version
echo ""
echo -e "\033[34m --------- Updating you-get --------- \033[0m"
echo "https://github.com/soimort/you-get/archive/v${LATEST_VERSION}.zip"
curl -L -o you-get.zip "https://github.com/soimort/you-get/archive/v${LATEST_VERSION}.zip"

echo ""
echo "Installing..."
unzip -q you-get.zip
rm -rf "$HOME/Library/Application Support/MoonPlayer/you-get"
mv you-get-* "$HOME/Library/Application Support/MoonPlayer/you-get"
chmod +x "$HOME/Library/Application Support/MoonPlayer/you-get/you-get"
rm -f you-get.zip

echo ""
echo -e "\033[34m ---------------- End ---------------- \033[0m"
