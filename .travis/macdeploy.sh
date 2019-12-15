#!/bin/bash

# Run macdeployqt
/usr/local/opt/qt/bin/macdeployqt $1 -qmldir=$2

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

# Check again
echo "Check: Empty is ok"
otool -L $1/Contents/Frameworks/*.dylib $1/Contents/MacOS/* | grep /usr/local
exit 0
