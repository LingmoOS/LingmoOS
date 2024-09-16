#!/bin/bash
if [ -z "$1" ]; then
    echo "A parameter is required.(eg: enable|disable|save)"
    exit 0
fi
. /usr/lib/deepin-graphics-driver-manager/common.sh
case $1 in
    "enable")
    overlayroot_enable
    ;;
    "disable")
    overlayroot_disable
    ;;
    "save")
    overlayroot_save
    ;;
    *)
    echo "paramers error!!!"
    exit 1
    ;;
esac

exit 0