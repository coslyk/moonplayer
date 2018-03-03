#!/bin/sh

sh upgrade-you-get.sh
sh upgrade-ykdl.sh

OS_NAME=`uname -s`
if [ "$OS_NAME" = 'Linux' ]; then
    echo "Press enter to continue"
    read LINE
fi
