#!/bin/bash

. /usr/lib/deepin-graphics-driver-manager/common.sh

cleanWorking

if [[ -n "${isInOverlayRoot}" ]]; then
    [ -e "${OVERLAY_LOWDIR}/${INSTALLER_DESKTOP_FILE_DEST}" ] && /usr/sbin/overlayroot-chroot rm -rf $INSTALLER_DESKTOP_FILE_DEST
    [ -e "${OVERLAY_LOWDIR}/${TEST_INSTALLER_DESKTOP_FILE_DEST}" ] && /usr/sbin/overlayroot-chroot rm -rf $TEST_INSTALLER_DESKTOP_FILE_DEST
    recovery_initramfs
else
    [ -e "${INSTALLER_DESKTOP_FILE_DEST}" ] && rm -rf $INSTALLER_DESKTOP_FILE_DEST
    [ -e "${TEST_INSTALLER_DESKTOP_FILE_DEST}" ] && rm -rf $TEST_INSTALLER_DESKTOP_FILE_DEST
fi

exit 0
