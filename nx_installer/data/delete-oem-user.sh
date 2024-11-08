#!/bin/bash

set -e

remove_service() {
    if [ -f /lib/systemd/system/delete-oem-user.service ]; then
        rm -f /lib/systemd/system/delete-oem-user.service
    fi
}

delete_oem_user() {
    if getent passwd 'oem' >/dev/null; then
        if [ -f /etc/sudoers.d/oem ]; then
            rm -f /etc/sudoers.d/oem
        fi
        pkill -u 'oem' || true
        userdel --force --remove 'oem' || true
    fi
}

remove_user_dir() {
    for user in $(ls /home); do
        if ! getent passwd ${user} >/dev/null; then
            rm -rf /home/${user}/.config || true
        fi
        rmdir /home/${user} || true
    done
}

remove_packages()
{
    # remove package list
    packages_list_remove=(
        lingmo-os-installer
        # casper
        lingmo-installer-wayland
    )

    for i in ${packages_list_remove[*]}; do
        if dpkg -l | grep -q $i; then
            apt purge -y $i || true
        fi
    done
}

copy_log() {
    if [ -f /tmp/oem.log ]; then
        mkdir -p /var/log/installer
        cp /tmp/oem.log /var/log/installer
    fi
}

restart_lightdm() {
    systemctl restart lightdm
}

set_test_mode_lightdm() {
    if grep -q 'test-mode=true' /usr/share/lingmo-os-installer/ky-installer.cfg; then
        restart_lightdm
    fi
}

set_990_9a0_lightdm() {
    if egrep -qi 'kirin.?9[09]0' /proc/cpuinfo; then
        systemctl restart lightdm
    fi
}
#临时解决3a5000 oem无法进入桌面问题
set_3a5000_lightdm() {
    if egrep -qi '3A5000' /proc/cpuinfo; then
        systemctl restart lightdm
    fi
}



restart_lingmo_support_kirin() {
  if [[ -f /lib/systemd/system/lingmo-support-kirin.service ]]; then
    systemctl restart lingmo-support-kirin
  fi
}

# delete-oem-user.service 是拷贝的，需要单独移除
remove_service >>/tmp/oem.log

remove_packages >>/tmp/oem.log

set_990_9a0_lightdm

delete_oem_user >>/tmp/oem.log

remove_user_dir >>/tmp/oem.log

copy_log

set_test_mode_lightdm

#set_3a5000_lightdm

restart_lingmo_support_kirin
#restart_lightdm



