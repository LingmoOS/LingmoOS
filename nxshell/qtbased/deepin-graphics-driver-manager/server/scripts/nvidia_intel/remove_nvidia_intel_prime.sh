#!/bin/bash

/usr/lib/deepin-graphics-driver-manager/nvidia/remove_nvidia_closesource.sh

if [ -f "/etc/X11/xorg.conf.d/70-nvidia.conf" ]; then
    rm -f /etc/X11/xorg.conf.d/70-nvidia.conf
fi

if [ -f "/etc/lightdm/display_setup.sh" ]; then
    rm -f /etc/lightdm/display_setup.sh
fi

sed -i '/^display-setup-script=\/etc\/lightdm\/display_setup.sh/d' /etc/lightdm/lightdm.conf