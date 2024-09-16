#!/bin/bash

if [ "$(id -u)" -ne "0" ];then
    echo "Need root privileges."
    exit 1
fi

. /usr/lib/deepin-graphics-driver-manager/common.sh

rm -rf /etc/modprobe.d/bumblebee.conf


packages=(
    "bumblebee"
    "bumblebee-nvidia"
    "primus"
    "primus-libs"
    "bbswitch-dkms"
    "nvidia-driver"
    "nvidia-alternative"
    "xserver-xorg-video-nvidia"
)

package_remove "${packages[*]}" "${#packages[*]}"

remove_nvidia
