#!/bin/bash

set -e

cfgFile=/usr/share/lingmo-os-installer/ky-installer.cfg
logFile=/var/log/installer/lingmo-os-installer-oem.log

if [[ -f /etc/xdg/autostart/oem-qc.desktop ]]; then
      sudo rm -f /etc/xdg/autostart/oem-qc.desktop
fi

userName=
timeZone=
password=
autoLogin=
userDefGroup=

msg() {
    echo "$(date +'%Y-%m-%d %H:%M:%S') $*" >> $logFile
}

get_value()
{
    indexStr=$1
    valueStr=

    valueStr=$(grep "^$indexStr=" "$cfgFile")
    valueStr=${valueStr##*=}
    echo "${valueStr}"
}

get_value_bytearray() {
    local key=$1
    local value
    value=$(grep "^$key" "$cfgFile")

    # TODO, 连续空格有问题
    if ! egrep -q '^password=.?@ByteArray' $cfgFile; then
        echo "${value#*=}"
        return 0
    fi

    value1=$(echo "${value#*\(}")
    echo "${value1%)*}"
}

set_timezone()
{
    timeZone=$(get_value "timezone")
    #msg "Set timezone to $timeZone"
    ln -sf /usr/share/zoneinfo/$timeZone /etc/localtime

    hwclock -w
}

set_user()
{
    userName=$(get_value "username")
    password=$(get_value_bytearray "password")
    autoLogin=$(get_value "autologin")
    KUID=1000

    #msg "Set user $userName"
    useradd -m -s /bin/bash -u "${KUID}" "${userName}"
    usermod -c "${userName}" "${userName}"
    echo "$userName:$password" | chpasswd
}

add_groups()
{
    userName=$(get_value "username")
    userDefGroup=(adm cdrom sudo dip plugdev lpadmin sambashare debian-tor libvirtd lxd)
    for item in ${userDefGroup[*]}; do
        adduser $userName $item || true
    done
}

set_auto_login()
{
    if [[ "${autoLogin}" == "1" ]]; then
        if [[ -d /etc/lightdm ]]; then
            LightDMCustomFile=/etc/lightdm/lightdm.conf
            autoLoginParam="autologin-guest=false\n\
autologin-user=$userName\n\
autologin-user-timeout=0"

            if ! grep -qs '^autologin-user' ${LightDMCustomFile}; then
                if ! grep -qs '^\[Seat:\*\]' ${LightDMCustomFile}; then
                    echo '[Seat:*]' >>${LightDMCustomFile}
                fi
                sed -i "s/\[Seat:\*\]/\[SeatDefaults]\n${AutologinParameters}/" ${LightDMCustomFile}
            # oem config scenario
            else
                #sed -i "s/^\(\(str  *\)\?autologin-user\)=.*$/\1=${USERNAME}/g;" /etc/lightdm/lightdm.conf
                sed -i "s/^autologin-user=.*$/autologin-user=${userName}/g" ${LightDMCustomFile}
            fi
        fi
    else
        if [[ -f /etc/lightdm/lightdm.conf ]]; then
            rm -f /etc/lightdm/lightdm.conf
        fi
    fi
}

set_language()
{
    language=$(get_value "language")
    echo "zh_CN.UTF-8 UTF-8
en_US.UTF-8 UTF-8
zh_HK.UTF-8 UTF-8
mn_MN.UTF-8 UTF-8
bo_CN.UTF-8 UTF-8
kk_KZ.UTF-8 UTF-8
ug_CN.UTF-8 UTF-8
ky_KG.UTF-8 UTF-8" > /etc/locale.gen

    #msg "Set language $language"
    echo "LANG=$language.UTF-8" > /etc/default/locale
    if [[ "$language" == "zh_CN" ]]; then
        echo "LANGUAGE=\"zh_CN:zh\"" >> /etc/default/locale
    elif [[ "${language}" = "bo_CN" ]]; then
    	echo "LANGUAGE=\"bo_CN:zh_CN:zh\"" >>/etc/default/locale
    elif [[ "${language}" = "zh_HK" ]]; then
        echo "LANGUAGE=\"zh_HK:zh_CN:zh\"" >>/etc/default/locale
    elif [[ "${language}" = "mn_MN" ]]; then
        echo "LANGUAGE=\"mn_MN\"" >>/etc/default/locale
    elif [[ "${language}" = "kk_KZ" ]]; then
        echo "LANGUAGE=\"kk_KZ\"" >>/etc/default/locale
    elif [[ "${language}" = "ug_CN" ]]; then
        echo "LANGUAGE=\"ug_CN\"" >>/etc/default/locale
    elif [[ "${language}" = "ky_KG" ]]; then
        echo "LANGUAGE=\"ky_KG\"" >>/etc/default/locale
    fi

    locale-gen

    if [[ -f /lib/systemd/system/accounts-daemon.service ]]; then
        systemctl restart accounts-daemon || true
    fi
}

set_hostname()
{
    HOSTNAME=$(get_value "hostname")
    echo "${HOSTNAME}" >/etc/hostname
    hostnamectl set-hostname "${HOSTNAME}"

    echo -e "127.0.0.1\tlocalhost

127.0.1.1\t${HOSTNAME}
# The following lines are desirable for IPv6 capable hosts
::1     ip6-localhost ip6-loopback
fe00::0 ip6-localnet
ff00::0 ip6-mcastprefix
ff02::1 ip6-allnodes
ff02::2 ip6-allrouters
" >/etc/hosts

  # bug 39277, 重启蓝牙服务
  if [[ -f /lib/systemd/system/bluetooth.service ]]; then
    /bin/systemctl restart bluetooth.service || true
  fi
}

remove_packages()
{
    # remove package list
    packages_list_remove=(
        lingmo-os-installer
        casper
        lingmo-installer-wayland
        ubiquity
    )
    
    for i in ${packages_list_remove[*]}; do
        if dpkg -l | grep -q $i; then
            msg "移除 $i"
            apt purge -y $i
        fi
    done
}

remove_oem_service() {
    if [[ -f /lib/systemd/system/oem-config.service ]]; then
        /bin/systemctl disable oem-config.service || true
        /bin/systemctl disable oem-config.target || true
        rm -f /lib/systemd/system/oem-config.*
        /bin/systemctl --no-block isolate graphical.target || true
    fi

 #   if [[ -f /etc/xdg/autostart/oem-qc.desktop ]]; then
 #       rm -f /etc/xdg/autostart/oem-qc.desktop
 #   fi

    if [[ -f /usr/share/lingmo-os-installer/data/oem-qc.desktop ]]; then
        rm -f /usr/share/lingmo-os-installer/data/oem-qc.desktop
    fi
    if [[ -f /etc/xdg/autostart/lingmo-os-installer.desktop ]]; then
        rm -f /etc/xdg/autostart/lingmo-os-installer.desktop
    fi

    if [[ -f /usr/share/lingmo-os-installer/data/oem-unlock.desktop ]]; then
        rm -f /usr/share/lingmo-os-installer/data/oem-unlock.desktop
    fi

    if [[ -f /etc/xdg/autostart/oem-unlock.desktop ]]; then
        rm -f /etc/xdg/autostart/oem-unlock.desktop
    fi
    
    if [[ -f /usr/sbin/oem-unlock ]]; then
        rm -f /usr/sbin/oem-unlock
    fi

}

delete_oem_user() {
    if getent passwd 'oem' >/dev/null; then
        if [[ -f /etc/sudoers.d/oem ]]; then
            rm -f /etc/sudoers.d/oem
        fi
        pkill -u 'oem' || true
        userdel --force --remove 'oem' || true
    fi
}

set_test_mode() {
    if grep -q 'test-mode=true' ${cfgFile}; then
        userName=$(get_value "username")
        # sudo 免密
        echo "${userName} ALL=(ALL) NOPASSWD: ALL" >/etc/sudoers.d/${userName}

        if [[ ! -z "$DISPLAY" ]]; then
            xset s off -dpms
        fi
    fi
}

restart_lightdm()
{
    systemctl restart lightdm
}

set_language >>${logFile}
set_timezone >>${logFile}
set_hostname >>${logFile}
set_user >>${logFile}
add_groups >>${logFile}

set_test_mode >>${logFile}

set_auto_login >>${logFile}
remove_oem_service >>${logFile}
systemctl start delete-oem-user.service

case $1 in
'user')
	set_user
	;;
'host')
	set_hostname
	;;
'time')
	set_timezone
	;;
'locale')
	set_language
	;;
'autologin')
	set_auto_login
	;;
'addgroup')
	add_groups
	;;
'unload')
	rm_ky_installer
	;;
'logout')
	restart_lightdm
	;;
esac
