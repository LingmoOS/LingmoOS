#!/bin/bash

if [ "$(id -u)" -ne "0" ];then
    echo "Need root privileges."
    exit 1
fi

. /usr/lib/deepin-graphics-driver-manager/common.sh

# 0 ------ default is glamor
# 1 ------ use SNA accel method
# 2 ------ use UXA accel method

export INTEL_XORG_CONF="/etc/X11/xorg.conf.d/20-intel.conf"

set_intel_accel_sna() {
echo 'Section "Device"
Identifier "Intel Graphics"
Driver "intel"
Option "AccelMethod" "sna"
#Option "PageFlip" "False"
#Option "TearFree" "True"
EndSection' | tee $INTEL_XORG_CONF
}

set_intel_accel_uxa() {
echo 'Section "Device"
Identifier "Intel Graphics"
Driver "intel"
Option "AccelMethod" "uxa"
#Option "PageFlip" "False"
#Option "TearFree" "True"
EndSection' | tee $INTEL_XORG_CONF
}

packages=(
    "xserver-xorg-core"
    "xserver-xorg-input-all"
    "xserver-xorg-video-intel"
)

package_install "${packages[*]}" "${#packages[*]}"

#根据需求调整显卡的加速方式，从而达到最优化
case $1 in
    0)
        # it is glamor mode after config files and drivers of other modes has been removed
        echo "Using default glamor accel method overlay"
        ;;
    1)
        set_intel_accel_uxa
        ;;
    2)
        set_intel_accel_sna
        ;;
    *)
        rm /etc/X11/xorg.conf.d/20-intel.conf
        [ -f /etc/X11/xorg.conf ] && rm /etc/X11/xorg.conf
        echo "You need to set an accel method,default glamor (overlay)"
        ;;
esac
