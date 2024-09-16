#!/bin/bash

if [ "$(id -u)" -ne "0" ];then
    echo "Need root privileges."
    exit 1
fi

. /usr/lib/deepin-graphics-driver-manager/common.sh


# 0 ------ default is glamor
# 1 ------ use SNA accel method
# 2 ------ use UXA accel method


packages=(
    "xserver-xorg-video-intel"
)

remove_config_driver_overlay() {
    rm /etc/X11/xorg.conf.d/20-intel.conf
    package_remove "${packages[*]}" "${#packages[*]}"
}

case $1 in
    0)
        echo "Nothing to remove..."
        ;;
    1|2)
        remove_config_driver_overlay
        ;;
    *)
        echo "You need to set an accel method to remove, do nothing..."
        ;;
esac
