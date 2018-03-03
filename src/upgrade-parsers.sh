#!/bin/sh

sh "${0%/*}/upgrade-you-get.sh"
sh "${0%/*}/upgrade-ykdl.sh"

OS_NAME=`uname -s`
if [ "$OS_NAME" = 'Linux' ]; then
    echo "Press enter to continue"
    read LINE
fi
