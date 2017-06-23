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


# Get latest you-get version
get_latest_version() {
    export PYTHONIOENCODING=utf8
    curl -s 'https://api.github.com/repos/rosynirvana/you-get/branches/master' | \
        python -c "import sys, json; print json.load(sys.stdin)['commit']['sha']"
}

LATEST_VERSION=`get_latest_version`
if [ -n "$LATEST_VERSION" ]; then
    echo "Latest version: $LATEST_VERSION"
else
    echo 'Error: Cannot get the latest version of you-get. Please try again later.'
    exit 0
fi


# Get current you-get version
VERSION_FILE="$HOME/Library/Application Support/MoonPlayer/you-get-version.txt"
if [ -e "$VERSION_FILE" ]; then
    CURRENT_VERSION=`cat "$VERSION_FILE"`
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
echo "https://github.com/rosynirvana/you-get/archive/master.zip"
curl -L -o you-get.zip "https://github.com/rosynirvana/you-get/archive/master.zip"

echo ""
echo "Installing..."
unzip -q you-get.zip
rm -rf "$HOME/Library/Application Support/MoonPlayer/you-get"
mv you-get-* "$HOME/Library/Application Support/MoonPlayer/you-get"
chmod +x "$HOME/Library/Application Support/MoonPlayer/you-get/you-get"
rm -f you-get.zip


# Save version info
echo "$LATEST_VERSION" > "$VERSION_FILE"

echo ""
echo -e "\033[34m ---------------- End ---------------- \033[0m"
