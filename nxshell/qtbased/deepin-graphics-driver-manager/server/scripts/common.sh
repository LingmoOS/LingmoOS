#!/bin/bash

export WORKING_DIR_G=/usr/lib/deepin-graphics-driver-manager/working-dir
export TEST_IN_OVERLAY_G=$WORKING_DIR_G/test_in_overlay_flag
export REMOVE_OLD_G=$WORKING_DIR_G/remove_old.sh
export INSTALL_NEW_G=$WORKING_DIR_G/install_new.sh
export CONFIG_FILE_G=$WORKING_DIR_G/config.conf
export PRIME_GLTEST_G=${WORKING_DIR_G}/deepin-prime-gltest
export REAL_INSTALLE_DESKTOP=/etc/xdg/autostart/deepin-gradvrmgr-installer.desktop

export INSTALLER_DESKTOP_FILE_SOURCE=/usr/lib/deepin-graphics-driver-manager/deepin-gradvrmgr-installer.desktop
export INSTALLER_DESKTOP_FILE_DEST=/etc/xdg/autostart/deepin-gradvrmgr-installer.desktop
export TEST_INSTALLER_DESKTOP_FILE_SOURCE=/usr/lib/deepin-graphics-driver-manager/deepin-gradvrmgr-test-installer.desktop
export TEST_INSTALLER_DESKTOP_FILE_DEST=/etc/xdg/autostart/deepin-gradvrmgr-test-installer.desktop

export is10GENERATIONS_CPU=false

export OVERLAY_LOWDIR=/media/root-ro/

export GLTEST_FLAG=/usr/lib/deepin-graphics-driver-manager/working-dir/dgradvrmgr_gltest_flag
export OVERLAYROOT_IMAGE=$WORKING_DIR_G/overlayroot.img
export isInOverlayRoot=$(grep -Em1 "^overlayroot / overlay |^overlay-root / overlay " /proc/mounts) || isInOverlayRoot=
export DEBIAN_FRONTEND=noninteractive

#define error exit code
export COMMON_ERROR=1
export APT_UPDATE_ERROR=2
export PURGE_PACKAGE_ERROR=3
export INSTALL_PACKAGE_ERROR=4
export NETWORK_CONNECTION_ERROR=5
export OVERLAYROOT_ENABLE_ERROR=6
export OVERLAYROOT_DISABLE_ERROR=7
export OVERLAYROOT_SAVE_ERROR=8
export OVERLAYROOT_MOUNT_ERROR=9
export GLTEST_RUNNING_ERROR=10

OVERLAYROOT_CONF=/etc/overlayroot.conf
LOOP_DEV=/dev/loop0

overlayroot_disable() {
    if [[ -n "${isInOverlayRoot}" ]]; then
         overlayroot-chroot sed -i 's:overlayroot=".*":overlayroot="":' ${OVERLAYROOT_CONF}
        [ -e "${OVERLAYROOT_IMAGE}" ] && overlayroot-chroot rm -f ${OVERLAYROOT_IMAGE}
    else
        sed -i 's:overlayroot=".*":overlayroot="":' ${OVERLAYROOT_CONF}
        [ -e "${OVERLAYROOT_IMAGE}" ] && rm -f ${OVERLAYROOT_IMAGE}
    fi
}

overlayroot_enable() {
    if [[ -n "${isInOverlayRoot}" ]]; then
        echo "overlayroot is enabled already"
    else
        [ -e "${OVERLAYROOT_IMAGE}" ] && rm -f ${OVERLAYROOT_IMAGE}
        dd if=/dev/zero of=${OVERLAYROOT_IMAGE} bs=1MiB count=1536
        mkfs.ext4 ${OVERLAYROOT_IMAGE} && sed -i "s:overlayroot=".*":overlayroot=\"device\:dev=\/dev\/loop0,recurse=0\":" ${OVERLAYROOT_CONF}
    fi
}

overlayroot_save() {
    #overlayroot-chroot cp -f /run/overlayroot/overlayroot.img ${OVERLAYROOT_IMAGE}
    POSTOS=`cat /proc/mounts | awk '{if ($2 == "/media/root-ro") print $1}'`
    mount -o remount,rw $POSTOS /media/root-ro
    sync
    cp -f /run/overlayroot/overlayroot.img ${OVERLAY_LOWDIR}/${OVERLAYROOT_IMAGE}
}
 
cleanWorking() {
    if [[ -n "${isInOverlayRoot}" ]]; then
        [ -e "${OVERLAY_LOWDIR}/${TEST_IN_OVERLAY_G}" ] && /usr/sbin/overlayroot-chroot rm -rf $TEST_IN_OVERLAY_G
        [ -e "${OVERLAY_LOWDIR}/${REMOVE_OLD_G}" ] && /usr/sbin/overlayroot-chroot rm -rf $REMOVE_OLD_G
        [ -e "${OVERLAY_LOWDIR}/${INSTALL_NEW_G}" ] && /usr/sbin/overlayroot-chroot rm -rf $INSTALL_NEW_G
        [ -e "${OVERLAY_LOWDIR}/${GLTEST_FLAG}" ] && /usr/sbin/overlayroot-chroot rm -rf $GLTEST_FLAG
    else
        [ -e "${TEST_IN_OVERLAY_G}" ] && rm -rf $TEST_IN_OVERLAY_G
        [ -e "${REMOVE_OLD_G}" ] && rm -rf $REMOVE_OLD_G
        [ -e "${INSTALL_NEW_G}" ] && rm -rf $INSTALL_NEW_G
        [ -e "${GLTEST_FLAG}" ] && rm -rf $GLTEST_FLAG
       
    fi
     [ -e "${PRIME_GLTEST_G}" ] && rm -rf $PRIME_GLTEST_G
    overlayroot_disable
}


backup_initramfs(){
    if [[ -n "${isInOverlayRoot}" ]]; then
        cp -f /boot/initrd.img*  ${WORKING_DIR_G}
    fi

}

recovery_initramfs(){
    if [[ -n "${isInOverlayRoot}" ]]; then
        cp -f  ${WORKING_DIR_G}/initrd.img* /boot/
        sync
    fi
}

check_cpu() {
    cpu_mode=$(lscpu |grep "Model name:" |sed 's/Model name:[ \n \t ]*//g')
    echo "cpu mode: $cpu_mode"
    is_intel=$(echo ${cpu_mode}　|grep -i intel)
    if [[ ${is_intel} != "" ]]; then
        cpu_id=$(echo $cpu_mode |awk '{print $3}' |sed 's/.*-//g' |tr -d 'a-zA-Z')
        [ "$cpu_id" -ge "10700" ] && is10GENERATIONS_CPU=true
    fi
}

purge_intelgpu() {
    apt-get -y purge intelgpu-dkms || true
}

install_intelgpu() {
    apt-get -y install intelgpu-dkms || true
}

error_exit() {
    echo "Error: $1"
    cleanWorking
    modify_config "exit_code" "$2"
    sync
    exit "$2"
}

error_reboot() {
    echo "Error: $1"
    cleanWorking
    modify_config "exit_code" "$2"
    sync
    reboot
    exit "$2"
}

apt_update()
{
    loop=0
    while [ $loop -lt 20 ]
    do
        sleep 1
        let loop+=1
        apt-get update && (apt-get install --fix-missing || dpkg --configure -a)
        [ $? -eq 0 ] && break
    done
}

package_remove()
{
    set -e
    pkg_list=$1
    len=$2
    inital_ratio=15
    max_ratio=45
    index=0
    echo "PROGRESS:15"
    let range=${max_ratio}-${inital_ratio}
    for pkg in ${pkg_list[@]}
    do
        apt-get -y purge ${pkg};
        let index+=1
        ratio=$(($index*$range/$len))
        let ratio+=${inital_ratio}
        echo "PROGRESS:${ratio}"
    done

    apt-get autoremove -y
    echo "PROGRESS:50"
    set +e
}

package_install()
{
   set -e
   pkg_list=$1
   len=$2
    inital_ratio=50
    max_ratio=95
    index=0
    let range=${max_ratio}-${inital_ratio}
    for pkg in ${pkg_list[@]}
    do
        apt-get -y --reinstall --allow-downgrades install ${pkg};
        let index+=1
        ratio=$(($index*$range/$len))
        let ratio+=${inital_ratio}
        echo "PROGRESS:${ratio}"
    done
    update-initramfs -u
    echo "PROGRESS:99"
    set +e
}

check_network()
{
    loop=0
    while [ $loop -lt 5 ]
    do
        sleep 1
        let loop+=1
        ping -c 1 www.baidu.com > /dev/null 2>&1
        [ $? -eq 0 ] && break
    done
}

modify_config()
{
    key=$1
    value=$2
    if [[ -n "${isInOverlayRoot}" ]]; then
        overlayroot-chroot sed -i -E "s/(${key}=).*$/\1${value}/" $CONFIG_FILE_G
    else
        sed -i -E "s/(${key}=).*$/\1${value}/" $CONFIG_FILE_G
    fi
}

remove_nvidia()
{
    dpkg -l | grep nvidia | awk '{print $2}' | xargs apt-get purge -y
}
