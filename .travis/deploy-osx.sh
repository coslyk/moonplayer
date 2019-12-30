#!/bin/bash

# Move compiled bundle here
mv src/MoonPlayer.app .

# Bundle libraries
/usr/local/opt/qt/bin/macdeployqt MoonPlayer.app -qmldir=src/qml/

# Fix permissions
chown $USER $1/Contents/MacOS/*
chmod 755 $1/Contents/MacOS/*

# Fix dependencies of homebrewed libraries
ls $1/Contents/Frameworks/*.dylib $1/Contents/MacOS/* | while read FILENAME; do
    DEPLOYMENT=`otool -L "$FILENAME" | grep /usr/local`
    if [ -n "$DEPLOYMENT" ]; then
        echo "Parsing: $FILENAME"
        echo "$DEPLOYMENT" | while read OLD_PATH; do
            OLD_PATH=${OLD_PATH# *}
            OLD_PATH=${OLD_PATH%% *}
            OLD_NAME=${OLD_PATH##*/}
            NEW_PATH="@executable_path/../Frameworks/$OLD_NAME"
            REAL_PATH="$1/Contents/Frameworks/$OLD_NAME"
            echo "  Old path: $OLD_PATH"
            echo "  New path: $NEW_PATH"
            if [ -e "$REAL_PATH" ]; then
                echo "  Found: $REAL_PATH"
                install_name_tool -change "$OLD_PATH" "$NEW_PATH" "$FILENAME"
            else
                echo "  Not found"
            fi
            echo "  ----"
        done
        echo ""
    fi
done

# Compress to zip file
zip -9 -r MoonPlayer_${TRAVIS_TAG#v}_macOS.zip MoonPlayer.app