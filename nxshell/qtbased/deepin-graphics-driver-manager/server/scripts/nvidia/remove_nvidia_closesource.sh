#!/bin/bash

if [ "$(id -u)" -ne "0" ];then
    echo "Need root privileges."
    exit 1
fi

. /usr/lib/deepin-graphics-driver-manager/common.sh

packages=(
    "glx-alternative-nvidia"
    "nvidia-alternative"
)

package_remove "${packages[*]}" "${#packages[*]}"

remove_nvidia

exit 0
