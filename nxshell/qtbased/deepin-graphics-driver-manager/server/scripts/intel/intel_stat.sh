#!/bin/bash

# 0 ------ default is glamor
# 1 ------ use SNA accel method
# 2 ------ use UXA accel method

intel_accel_method_sna=`cat /var/log/Xorg.0.log | grep SNA`
intel_accel_method_uxa=`cat /var/log/Xorg.0.log | grep UXA`

if [ -n "$intel_accel_method_sna" ];then
	echo "now intel accel mode is : SNA"
	exit 2
elif [ -n "$intel_accel_method_uxa" ];then
	echo "now intel accel mode is : UXA"
	exit 1
else
	echo "now intel accel mode is : glamor(default)"
	exit 0
fi
