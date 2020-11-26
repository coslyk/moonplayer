#!/bin/bash

# Move compiled bundle here
mv src/MoonPlayer.app .
mv 3rdparty/moonplayer-hlsdl MoonPlayer.app/Contents/MacOS/
cp /usr/local/opt/ffmpeg-lite/bin/ffmpeg MoonPlayer.app/Contents/MacOS/

# Bundle libraries
/usr/local/opt/qt/bin/macdeployqt MoonPlayer.app -qmldir=src/qml/ -executable=MoonPlayer.app/Contents/MacOS/moonplayer-hlsdl

# Cut Qt size
rm -rf MoonPlayer.app/Contents/Frameworks/QtPdf.framework
rm -rf MoonPlayer.app/Contents/Frameworks/QtRemoteObjects.framework
rm -rf MoonPlayer.app/Contents/Frameworks/QtVirtualKeyboard.framework
rm -rf MoonPlayer.app/Contents/PlugIns/platforminputcontexts
rm -rf MoonPlayer.app/Contents/PlugIns/printsupport
rm -rf MoonPlayer.app/Contents/PlugIns/virtualkeyboard

# Fix permissions
chown $USER MoonPlayer.app/Contents/MacOS/*
chmod 755 MoonPlayer.app/Contents/MacOS/*

# Fix dependencies of homebrewed libraries
ls MoonPlayer.app/Contents/Frameworks/*.dylib MoonPlayer.app/Contents/MacOS/* | while read FILENAME; do
    DEPLOYMENT=`otool -L "$FILENAME" | grep /usr/local`
    if [ -n "$DEPLOYMENT" ]; then
        echo "Parsing: $FILENAME"
        echo "$DEPLOYMENT" | while read OLD_PATH; do
            OLD_PATH=${OLD_PATH# *}
            OLD_PATH=${OLD_PATH%% *}
            OLD_NAME=${OLD_PATH##*/}
            NEW_PATH="@executable_path/../Frameworks/$OLD_NAME"
            REAL_PATH="MoonPlayer.app/Contents/Frameworks/$OLD_NAME"
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
