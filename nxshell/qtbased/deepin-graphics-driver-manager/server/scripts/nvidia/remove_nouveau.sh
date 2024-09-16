#!/bin/bash

if [ "$(id -u)" -ne "0" ];then
    echo "Need root privileges."
    exit 1
fi

. /usr/lib/deepin-graphics-driver-manager/common.sh

packages=(
    "xserver-xorg-video-nouveau"
)

package_remove "${packages[*]}" "${#packages[*]}"

