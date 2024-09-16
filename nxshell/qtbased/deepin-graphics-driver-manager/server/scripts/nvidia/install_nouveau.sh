#!/bin/bash

if [ "$(id -u)" -ne "0" ];then
    echo "Need root privileges."
    exit 1
fi

. /usr/lib/deepin-graphics-driver-manager/common.sh

packages=(
    "xserver-xorg-input-all"
    "xserver-xorg-core"
    "xserver-xorg-video-nouveau"
    "nvidia-installer-cleanup"
    "update-glx"
    "glx-alternative-mesa"
    "libgl1-mesa-glx"
    "libgl1-mesa-glx:i386"
    "libglx-mesa0"
    "libglx-mesa0:i386"
    "glx-diversions"
)

package_install "${packages[*]}" "${#packages[*]}"

exit 0
