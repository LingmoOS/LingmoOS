#!/bin/bash

. /usr/lib/deepin-graphics-driver-manager/common.sh

if [[ -z "${isInOverlayRoot}" ]]; then
    rm  -f /etc/xdg/autostart/deepin-gradvrmgr-test-installer.desktop
    cp ${INSTALLER_DESKTOP_FILE_SOURCE} ${INSTALLER_DESKTOP_FILE_DEST} || error_reboot "Copy ${INSTALLER_DESKTOP_FILE_SOURCE} to ${INSTALLER_DESKTOP_FILE_DEST} failed" ${COMMON_ERROR}
    error_reboot "Overlayroot failed to mount for the first time" ${OVERLAYROOT_MOUNT_ERROR}
fi

# boot分区不被overlayroot保护，需要单独备份
backup_initramfs

/usr/sbin/overlayroot-chroot rm -f /etc/xdg/autostart/deepin-gradvrmgr-test-installer.desktop
/usr/sbin/overlayroot-chroot cp ${INSTALLER_DESKTOP_FILE_SOURCE} ${INSTALLER_DESKTOP_FILE_DEST} || error_reboot "Overlay-chroot copy ${INSTALLER_DESKTOP_FILE_SOURCE} to ${INSTALLER_DESKTOP_FILE_DEST} failed" ${COMMON_ERROR}

# 测试安装时，最小化窗口后，启动器点击无界面弹出问题
cp -f ${TEST_INSTALLER_DESKTOP_FILE_SOURCE} /usr/share/applications/deepin-graphics-driver-manager.desktop

#check network
check_network || error_reboot "The network is not working, please check the network connection" ${NETWORK_CONNECTION_ERROR}

#apt-get update
apt_update || error_reboot "Execute apt update failed" ${APT_UPDATE_ERROR}

#remove old driver
$REMOVE_OLD_G 
if [ $? != 0 ]; then
    recovery_initramfs
    error_reboot "Remove old driver failed" ${PURGE_PACKAGE_ERROR}
fi

#install old driver
$INSTALL_NEW_G 
if [ $? != 0 ]; then
    recovery_initramfs
    error_reboot "Install new driver failed" ${INSTALL_PACKAGE_ERROR}
fi

/usr/sbin/overlayroot-chroot touch ${GLTEST_FLAG} || error_reboot "Overlay-chroot touch ${GLTEST_FLAG} failed" ${COMMON_ERROR}

#save the upperdir
overlayroot_save || error_reboot "Overlayroot save failed" ${OVERLAYROOT_SAVE_ERROR}

overlayroot-chroot sed -i "s:overlayroot=".*":overlayroot=\"device\:dev=\/dev\/loop0,recurse=0\":" ${OVERLAYROOT_CONF}

# 判断前前台进程是否还存在（防止被强行退出）
killall -e -0 test-installer
if [ $? -ne 0 ]; then
    reboot
fi

exit 0
