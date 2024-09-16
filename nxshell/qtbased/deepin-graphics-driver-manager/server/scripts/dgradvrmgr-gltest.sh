#!/bin/bash
systemctl stop lightdm

# run this command again here to avoid disable overlayroot in service not work

. /usr/lib/deepin-graphics-driver-manager/common.sh

if [[ -z "${isInOverlayRoot}" ]]; then
    error_reboot "Overlayroot failed to mount for the second time" ${OVERLAYROOT_MOUNT_ERROR}
fi

overlayroot_disable
recovery_initramfs


if [[ -e ${PRIME_GLTEST_G} ]]; then
    /usr/bin/xinit ${PRIME_GLTEST_G}
else
    /usr/bin/xinit /usr/lib/deepin-graphics-driver-manager/gltest
fi

error_reboot "Gltest running failed" ${GLTEST_RUNNING_ERROR}