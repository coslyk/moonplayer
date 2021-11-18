#!/bin/sh

# abort on all errors
set -e

script=$(readlink -f "$0")

APPDIR=

while [ "$1" != "" ]; do
    case "$1" in
        --plugin-api-version)
            echo "0"
            exit 0
            ;;
        --appdir)
            APPDIR="$2"
            shift
            shift
            ;;
        *)
            echo "Invalid argument: $1"
            exit 1
            ;;
    esac
done

if [ "$APPDIR" == "" ]; then
    exit 1
fi

mkdir -p "$APPDIR"


echo "Add hook"
HOOKSDIR="$APPDIR/apprun-hooks"
HOOKFILE="$HOOKSDIR/linuxdeploy-plugin-qtfix.sh"
mkdir -p "$HOOKSDIR"
cat > "$HOOKFILE" <<\EOF
export APPDIR="${APPDIR:-"$(dirname "$(realpath "$0")")"}"
export QT_QPA_PLATFORM_PLUGIN_PATH=${APPDIR}/usr/plugins
export QML2_IMPORT_PATH=${APPDIR}/usr/qml
EOF
