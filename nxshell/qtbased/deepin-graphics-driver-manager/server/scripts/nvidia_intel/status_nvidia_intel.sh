#!/bin/bash

# exit 0 ------ Intel Glamor Accel Mode
# exit 1 ------ Intel Uxa Accel Mode
# exit 2 ------ Intel Sna Accel Mode
# exit 3 ------ Bumblebee Mode
# exit 3 ------ Prime Mode

bumblebee_exist=`dpkg -s bumblebee 2>/dev/null | grep "Status: install ok installed"`
#prime_exist=`dpkg -s deepin-nvidia-prime 2>/dev/null | grep "Status: install ok installed"`
intel_accel_method_sna=`cat /var/log/Xorg.0.log | grep SNA`
intel_accel_method_uxa=`cat /var/log/Xorg.0.log | grep UXA`

if [ -f "/etc/X11/xorg.conf.d/70-nvidia.conf" ]; then
    echo "Prime Mode"
    exit 4
elif [ -n "$bumblebee_exist" ]; then
    echo "Bumblebee Mode"
    exit 3
elif [ -n "$intel_accel_method_sna" ];then
    echo "Intel Sna Accel Mode"
	exit 2
elif [ -n "$intel_accel_method_uxa" ];then
    echo "Intel Uxa Accel Mode"
    exit 1
else
    echo "Intel Glamor Accel Mode"
    exit 0
fi