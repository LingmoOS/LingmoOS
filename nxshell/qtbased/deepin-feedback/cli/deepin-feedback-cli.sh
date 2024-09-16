#!/bin/bash

# Copyright (C) 2015 Deepin Technology Co., Ltd.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.

###* Basic Configuration
LC_ALL=C

app_file="${0}"
app_name="$(basename $0)"

real_home="${HOME}"
if [ "${PKEXEC_UID}" ]; then
    SUDO_USER=`getent passwd ${PKEXEC_UID}| cut -d: -f1`
fi

if [ "${SUDO_USER}"  ]; then
    real_home="/home/${SUDO_USER}"
fi

opt_sliceinfo_func_list=("sliceinfo_basic")
opt_syslog_include=()

pkgcmd=(dpkg -l)

###* Help Functions
msg() {
    local mesg=${1}; shift
    printf "${GREEN}==>${ALL_OFF}${BOLD} ${mesg}${ALL_OFF}\n" "${@}" >&2
}

msg2() {
    local mesg=${1}; shift
    printf "${BLUE}  ->${ALL_OFF}${BOLD} ${mesg}${ALL_OFF}\n" "${@}" >&2
}

warning() {
    local mesg=${1}; shift
    printf "${YELLOW}==> WARNING:${ALL_OFF}${BOLD} ${mesg}${ALL_OFF}\n" "${@}" >&2
}

msg_title() {
    local msg="${1}"; shift
    printf "\n# ${msg}\n\n" ${@}
}

msg_code() {
    printf "%s" "${@}" | sed 's/^/    /'
    printf "\n"
}

setup_color_message() {
    unset ALL_OFF BOLD BLUE GREEN RED YELLOW
    if [[ -t 2 && $USE_COLOR != "n" ]]; then
        # prefer terminal safe colored and bold text when tput is supported
        if tput setaf 0 &>/dev/null; then
            ALL_OFF="$(tput sgr0)"
            BOLD="$(tput bold)"
            BLUE="${BOLD}$(tput setaf 4)"
            GREEN="${BOLD}$(tput setaf 2)"
            RED="${BOLD}$(tput setaf 1)"
            YELLOW="${BOLD}$(tput setaf 3)"
        else
            ALL_OFF="\e[0m"
            BOLD="\e[1m"
            BLUE="${BOLD}\e[34m"
            GREEN="${BOLD}\e[32m"
            RED="${BOLD}\e[31m"
            YELLOW="${BOLD}\e[33m"
        fi
    fi
}
setup_color_message

get_self_funcs() {
    grep -o "^${1}.*()" "${app_file}" | sed "s/^\(.*\)()/\1/" | sort
}

get_category_funcs() {
    get_self_funcs "category_" | sed 's/category_/        /g'
}

get_sliceinfo_funcs() {
    get_self_funcs "sliceinfo_" | sed 's/sliceinfo_/        /g'
}

grep_block() {
    local keyword="$1"; shift
    local files=$@
    awk -v keyword="${keyword}" 'BEGIN{RS="\n\n"; n=0} $0 ~ keyword{print ""; print; n++} END{print n, "of", NR, "matched", keyword}' $files
}

is_cmd_exists() {
    if type -a "${1}" &>/dev/null; then
        return 0
    else
        return 1
    fi
}

run() {
    if is_cmd_exists "${1}"; then
        "${@}" 2>&1
    else
        printf "command not found: %s" "${1}"
    fi
}

# collect_file <category> <files...>
get_log_path() {
    local _dir="${1}"; shift
    length=`jq --arg _dir $_dir "."$_dir"|length" /var/lib/deepin-feedback/feedback_logpath.json`
    _length=`expr $length - 1`
    for (( i=0; i<=$_length; i++  ))
    do
        path=`jq -r --arg _dir $_dir --arg i $i "."$_dir"[$i]" /var/lib/deepin-feedback/feedback_logpath.json`
        collect_file $_dir $path
    done
}

collect_file() {
    local category="${1}"; shift
    for f in ${@}; do
        do_collect_file "${category}" "${f}"
    done
}
do_collect_file() {
    local category="${1}"; shift
    local child_dest_dir="${dest_dir}/${category}/$(dirname "${1}")"
    local source_file="$(echo "${1}" | sed "s#^~#${real_home}#")"
    if [ -e "${source_file}" ]; then
        mkdir -p "${child_dest_dir}"
        cp -fR -t "${child_dest_dir}" "${source_file}"
    else
        msg2 "collect_file: ignore ${source_file}"
    fi
}

###* Functions to Get Certain System Information
sliceinfo_basic() {
    msg_title "Linux Release"
    msg_code "$(run lsb_release -a 2>&1)"

    if [ -f /etc/deepin-version ]; then
        msg_title "Deepin Release"
        msg_code "$(run cat /etc/deepin-version)"
    fi

    msg_title "Linux Kernel"
    msg_code "$(run uname -a)"

    msg_title "Installed Deepin Packages"
    msg_code "$(run ${pkgcmd[@]} | grep -i -e 'deepin-' -e '-deepin' -e 'dde-' -e '-dde' -e 'lastore')"

    msg_title "Computer Model"
    msg_code "$(run do_sliceinfo_basic_computer_model)"

    msg_title "Locale Specific Information"
    msg_code "$(run locale)"
}
do_sliceinfo_basic_computer_model() {
    # need root permission
    for f in /sys/class/dmi/id/*; do
        if [ -f "${f}" -a -r "${f}" ]; then
            printf "%s:\t%s\n" $f "$(cat $f)"
        fi
    done
}

sliceinfo_service() {
    if is_cmd_exists systemctl; then
        msg_title "Systemd Services"
        msg_code "$(run systemctl list-unit-files)"
    fi
    if is_cmd_exists initctl; then
        msg_title "Sysvinit Services"
        msg_code "$(run initctl list | column -t)"
    fi
}

sliceinfo_env() {
    env
}

sliceinfo_package() {
    if is_cmd_exists dpkg; then
        msg_title "DPKG Packages"
        msg_code "$(run dpkg -l)"
    fi
    if is_cmd_exists pacman; then
        msg_title "Pacman Packages"
        msg_code "$(run pacman -Qs)"
    fi
}

sliceinfo_device() {
    msg_title "CPU"
    msg_code "$(run cat /proc/cpuinfo)"

    msg_title "Memory"
    msg_code "$(run free -t -h)"
    msg_code "$(run cat /proc/meminfo)"

    msg_title "USB Devices"
    msg_code "$(run lsusb)"
    msg_code "$(run usb-devices)"

    msg_title "PCI Devices"
    msg_code "$(run lspci -vvnn)"

    msg_title "Hardware Lister(lshw)"
    msg_code "$(run lshw)" # need root permission
    msg_title "SMBIOS/DMI (dmidecode)"
    msg_code "$(run dmidecode)" # need root permission
}

sliceinfo_driver() {
    msg_title "Loaded Drivers"
    msg_code "$(run lsmod)"

    if [ -f /etc/modules ]; then
        msg_title "Driver Modules File"
        msg_code "$(run cat /etc/modules)"
    fi

    msg_title "Driver Blacklist File"
    msg_code "$(run do_sliceinfo_driver_blacklist)"

    msg_title "Installed Driver Packages"
    msg_code "$(run ${pkgcmd[@]} | grep -e driver -e catalyst -e nvidia -e drm -e mesa)"
}
do_sliceinfo_driver_blacklist() {
    for f in /etc/modprobe.d/*; do
        printf "${f}\n"
    done
    printf "\n\n"
    for f in /etc/modprobe.d/*; do
        printf "\n## ${f}\n"
        cat "${f}"
    done
}

sliceinfo_kernel() {
    dmesg
}

sliceinfo_audio() {
    msg_title "Audio Devices"
    msg_code "$(run lspci -vvnn | grep_block 'Audio')"

    msg_title "PulseAudio Version"
    msg_code "$(run pulseaudio --version)"

    msg_title "PulseAudio Configurations"
    msg_code "$(run pulseaudio --dump-conf)"

    msg_title "PulseAudio Modules"
    msg_code "$(run pulseaudio --dump-modules)"

    msg_title "PulseAudio Resample Methods"
    msg_code "$(run pulseaudio --dump-resample-methods)"
}

show_prop() {
    echo -n "$1: "
    cat $1
}

sliceinfo_backlight(){
    for i in /sys/class/backlight/* ;do
        cd $i
	echo Backlight $(basename $i)
	for p in actual_brightness brightness max_brightness type bl_power; do
	    show_prop $p
	done
	# show EDID
	echo EDID:
	xxd device/edid
    done
}

sliceinfo_battery(){
    msg_title "Battery Info"
    for dir in `ls /sys/class/power_supply/`
    do
        if [ -d /sys/class/power_supply/$dir ]
        then
            if [ -f /sys/class/power_supply/$dir/uevent ]
            then
                msg_code "/sys/class/power_supply/$dir/uevent"
                msg_code "$(run cat /sys/class/power_supply/$dir/uevent)"
                msg_code "        "
            fi
        fi
    done
    msg_code "$(run upower -d)"
    msg_code "        "
    gdbus call -y -d com.deepin.system.Power -o /com/deepin/system/Power -m com.deepin.system.Power.GetBatteries
    msg_code "$(run gdbus introspect -y -d com.deepin.system.Power -o /com/deepin/system/Power -rp)"

}
sliceinfo_video() {
    msg_title "Video Devices"
    msg_code "$(run lspci -vvnn | grep_block 'VGA ')"
    msg_code "$(run lspci -vvnn | grep_block '3D controller')"

    msg_title "Video Driver Packages"
    msg_code "$(run ${pkgcmd[@]} | grep -e xorg-video -e catalyst -e nvidia -e drm -e mesa)"

    msg_title "Xrandr Infromation"
    msg_code "$(run xrandr --verbose)"
    msg_title "Backlight Info"
    msg_code "$(sliceinfo_backlight)"
}

sliceinfo_network() {
    msg_title "Network Devices"
    if ! is_sliceinfo_device_will_run; then
        msg_code "$(run lshw -C network)" # need root permission
    fi
    msg_code "$(run lspci -vvnn | grep_block '[nN]etwork|[eE]thernet')"
    msg_code "$(run lsusb -v | grep_block '[nN]et|[eE]thernet')"

    msg_title "Network Status"
    msg_code "$(run ifconfig -v -a)"
    msg_code "$(run iwconfig)"

    msg_title "NetworkManager Configuration"
    msg_code "$(run cat /etc/NetworkManager/NetworkManager.conf)"

    msg_title "NetworkManager State"
    msg_code "$(run nmcli --version)"
    msg_code "$(run nmcli general status)"

    msg_title "NetworkManager Hotname"
    msg_code "$(run nmcli general hostname)"

    msg_title "NetworkManager Permissions"
    msg_code "$(run nmcli general permissions)"

    msg_title "NetworkManager Logging"
    msg_code "$(run nmcli general logging)"

    msg_title "NetworkManager Connections"
    msg_code "$(run nmcli connection)"

    msg_title "NetworkManager Devices"
    msg_code "$(run nmcli device status)"

    msg_title "NetworkManager Wireless Access Points"
    msg_code "$(run nmcli device wifi)"

    msg_title "ModemManager State"
    msg_code "$(run mmcli -L)"

    msg_title "Wireless Device Switches(rfkill)"
    msg_code "$(run rfkill list all)"

    if [ -f /etc/network/interfaces ]; then
        msg_title "Network Interface File"
        msg_code "$(run cat /etc/network/interfaces)"
    fi

    msg_title "DNS Configuration(resolv.conf)"
    msg_code "$(run cat /etc/resolv.conf)"

    msg_title "Route Table"
    msg_code "$(run route)"
}

sliceinfo_bluetooth() {
    msg_title "Bluetooth Devices"
    msg_code "$(run hciconfig -a)"
    msg_code "$(run lspci -vvnn | grep_block '[bB]luetooth')"
    msg_code "$(run lsusb | grep -i bluetooth)"

    msg_title "Loaded Bluetooth Drivers"
    msg_code "$(run lsmod | grep -e btusb -e bluetooth -e hidp -e rfcomm)"
}

sliceinfo_bootmgr() {
    msg_title "Boot Files"
    msg_code "$(run find /boot)"
    if [ -d "/sys/firmware/efi" ]; then
        msg_title "EFI Information"
        msg_code "$(run efibootmgr -v)"
    fi

    if [ ! "${arg_privacymode}" ]; then
        msg_title "Boot Info Script"
        msg_code "$(run bootinfoscript --stdout)" # need root permission
    fi
}

sliceinfo_disk() {
    if ! is_sliceinfo_device_will_run; then
        msg_title "Disk Devices"
        msg_code "$(run lshw -C disk -C storage)" # need root permission
    fi

    msg_title "Disk Partition Table"
    msg_code "$(run lsblk)"
	msg_title "Disk Partition Information"
	msg_code "$(run df -h)"
}

sliceinfo_fonts() {
    fc-list
}

sliceinfo_gsettings() {
    pkexec --user $SUDO_USER gsettings list-recursively | grep com.deepin
}

sliceinfo_fprint() {
    msg_title "Fprint Packages"
    msg_code "$(run dpkg -l | grep fprint)"

    msg_title "Fprint Devices"
    if [ -f /usr/lib/fprintd/fprintd ]; then
        msg_code "$(run qdbus --system --literal net.reactivated.Fprint /net/reactivated/Fprint/Manager net.reactivated.Fprint.Manager.GetDevices)"
    fi
}

sliceinfo_syslog() {
    # user journalctl firstly
    if is_cmd_exists journalctl; then
        if [ ${#opt_syslog_include[@]} -gt 0 ]; then
            journalctl --boot -1 2>/dev/null | grep -i ${opt_syslog_include[@]}
            journalctl --boot 0  2>/dev/null | grep -i ${opt_syslog_include[@]}
        else
            journalctl --boot -1 2>/dev/null
            journalctl --boot 0  2>/dev/null
        fi
    else
        if [ ${#opt_syslog_include[@]} -gt 0 ]; then
            cat /var/log/syslog{.1,} 2>/dev/null | grep -i ${opt_syslog_include[@]}
        else
            cat /var/log/syslog{.1,} 2>/dev/null
        fi
    fi
}
include_syslog_keyword() {
    local len=${#opt_syslog_include[@]}
    opt_syslog_include[$len]="-e"
    ((len++))
    opt_syslog_include[$len]="${1}"
}

include_sliceinfo() {
    # ignore repeated items
    for f in ${opt_sliceinfo_func_list[@]}; do
        if [ "${f}" = "sliceinfo_${1}" ]; then
            return 0
        fi
    done

    local len=${#opt_sliceinfo_func_list[@]}
    opt_sliceinfo_func_list[$len]="sliceinfo_${1}"
}
is_sliceinfo_device_will_run() {
    for f in ${opt_sliceinfo_func_list[@]}; do
        if [ "${f}" = "sliceinfo_device" ]; then
            return 0
        fi
    done
    return 1
}
exec_sliceinfo_funcs() {
    for f in ${opt_sliceinfo_func_list[@]}; do
        msg2 "executing ${f}..."
        case "${f}" in
            "sliceinfo_service")   "${f}"   >> "${file_service}";;
            "sliceinfo_env")       "${f}"   >> "${file_env}";;
            "sliceinfo_package")   "${f}"   >> "${file_package}";;
            "sliceinfo_bootmgr")   "${f}"   >> "${file_bootmgr}";;
            "sliceinfo_device")    "${f}"   >> "${file_device}";;
            "sliceinfo_gsettings") "${f}"   >> "${file_gsettings}";;
            "sliceinfo_driver")    "${f}"   >> "${file_driver}";;
            "sliceinfo_fonts")     "${f}"   >> "${file_fonts}";;
            "sliceinfo_kernel")    "${f}"   >> "${file_kernel}";;
            "sliceinfo_syslog")    "${f}"   >> "${file_syslog}";;
            "sliceinfo_fprint")    "${f}"   >> "${file_fprint}";;
            *)                     "${f}"   >> "${file_master}";;
        esac
    done
}

###* Categories
category_all() {
    # clean predefined options and execute all functions that could
    # get system information
    for f in $(get_category_funcs); do
        if [ "${f}" != "all" ]; then
            category_"${f}"
        fi
    done
    if [ ! "${arg_privacymode}" ]; then
        opt_syslog_include=()       # catch all syslog
    fi
}

category_dde() {
    # category_system include dde-control-center
    category_system
    subcategory_startdde
    subcategory_background
    subcategory_dde-desktop
    subcategory_dde-dock
    subcategory_dde-launcher
    subcategory_dde-offline-upgrader
}
subcategory_startdde() {
    include_sliceinfo "syslog"
    include_syslog_keyword "startdde"
}
subcategory_background() {
    include_sliceinfo "syslog"
    include_syslog_keyword "startdde"
    include_syslog_keyword "daemon/appearance"
}
subcategory_dde-desktop() {
    get_log_path "desktop"
}
subcategory_dde-dock() {
    include_sliceinfo "syslog"
    include_syslog_keyword "daemon/dock"
    get_log_path "dock"
}
subcategory_dde-launcher() {
    include_sliceinfo "syslog"
    include_syslog_keyword "daemon/launcher-daemon"
    get_log_path "launcher"
}

category_dde-control-center() {
    subcategory_bootmgr
    subcategory_background
    subcategory_display
    subcategory_bluetooth
    subcategory_network
    subcategory_pkglog

    # catch all syslog for dde-daemon
    include_sliceinfo "syslog"
    include_syslog_keyword "daemon/"

    include_sliceinfo "device"
    include_sliceinfo "fonts"
    include_sliceinfo "gsettings"
    get_log_path "dde_control_center"
}
subcategory_bootmgr() {
    if [ ! "${arg_privacymode}" ]; then
        include_sliceinfo "disk"
    fi
    include_sliceinfo "bootmgr"
    include_sliceinfo "syslog"
    include_syslog_keyword "daemon/grub"
    get_log_path "bootmgr"
}
subcategory_display() {
    include_sliceinfo "video"
    include_sliceinfo "syslog"
    include_syslog_keyword "startdde"
    include_syslog_keyword "daemon/display"
    get_log_path "display"
}
subcategory_bluetooth() {
    include_sliceinfo "bluetooth"
    include_sliceinfo "kernel"
    include_sliceinfo "driver"
    include_sliceinfo "syslog"
    include_syslog_keyword "daemon/bluetooth"
    include_syslog_keyword "bluetooth"
    get_log_path "bluetooth"
}
subcategory_network() {
    include_sliceinfo "network"
    include_sliceinfo "driver"
    include_sliceinfo "syslog"
    include_syslog_keyword "daemon/network"
    include_syslog_keyword "NetworkManager"
    include_syslog_keyword "ModemManager"
    include_syslog_keyword "wpa_supplicant"
    include_syslog_keyword "dhclient"
    include_syslog_keyword "dnsmasq"
    include_syslog_keyword "avahi-daemon"
    get_log_path "network"
}
subcategory_pkglog() {
    if [ "${arg_privacymode}" ]; then
        return
    fi

    include_sliceinfo "package"

    # debian
    if is_cmd_exists apt; then
        get_log_path "pkglog"
    fi

    # archlinux
    if is_cmd_exists pacman; then
        get_log_path "pkglog"
    fi
}

subcategory_dde-offline-upgrader() {
    include_sliceinfo "syslog"
    get_log_path "dde_offline_upgrader"
}

category_system() {
    category_dde-control-center
    subcategory_login
    include_sliceinfo "driver"
    include_sliceinfo "kernel"
    include_sliceinfo "video"
    include_sliceinfo "battery"
    if [ ! "${arg_privacymode}" ]; then
        include_sliceinfo "service"
    fi
    include_sliceinfo "fprint"
}
subcategory_login() {
    include_sliceinfo "syslog"
    include_syslog_keyword "startdde"
    include_syslog_keyword "daemon/display"
    if [ ! "${arg_privacymode}" ]; then
        get_log_path "login"
    fi
}

category_deepin-installer() {
    get_log_path "deepin_installer"
    if [ ! "${arg_privacymode}" ]; then
        include_sliceinfo "disk"
    fi
}
category_deepin-store() {
    subcategory_pkglog
    collect_file "deepin_store" $(find /var/lib/lastore/ -type f 2>/dev/null | grep -v safecache | grep -v tree)
    get_log_path "deepin_store"
    get_log_path "dde_control_center"
}
category_deepin-music() {
    get_log_path "deepin_music"
    include_sliceinfo "audio"
    include_sliceinfo "driver"
    include_sliceinfo "kernel"
}
category_deepin-movie() {
    get_log_path "deepin_movie"
    include_sliceinfo "video"
    include_sliceinfo "driver"
    include_sliceinfo "kernel"
}
category_deepin-boot-maker() {
    get_log_path "deepin_boot_maker"
}
category_deepin-screenshot() {
    get_log_path "deepin_screenshot"
}
category_deepin-terminal() {
    get_log_path "deepin_terminal"
}
category_deepin-translator() {
    get_log_path "deepin_translator"
}
category_deepin-cloud-print() {
    get_log_path "deepin_cloud_print"
}
category_deepin-cloud-scan() {
    get_log_path "deepin_cloud_scan"
}
category_deepin-file-manager() {
    get_log_path "dde_file_manager"
}
category_deepin-image-viewer() {
    get_log_path "deepin_image_viewer"
}
category_deepin-remote-assistance() {
    get_log_path "deepin_remote_assistance"
}
category_deepin-graphics-driver-manager() {
    get_log_path "deepin_graphics_driver_manager"
}
###* Main

# override variables
if is_cmd_exists pacman; then
    pkgcmd=(pacman -Q)
fi

if [ -f /etc/deepin-version ]; then
    distro_release="$(cat /etc/deepin-version | grep Version | awk -F'=' '{print $2}')"
fi
if is_cmd_exists lsb_release; then
    distro_name="$(lsb_release -s -i)"
    if [ -z "${distro_release}" ]; then
        distro_release="$(lsb_release -s -r)"
    fi

    # fix distro name to keep same with the IDs in bugzilla
    if [ "${distro_name}" = "Arch" ]; then
        distro_name="Arch Linux"
    fi
    if [ "${distro_name}" = "ManjaroLinux" ]; then
        distro_name="Manjaro"
    fi
    if [ "${distro_name}" = "SUSE LINUX" ]; then
        distro_name="SUSE"
    fi
else
    distro_name="Other"
    if [ -z "${distro_release}" ]; then
        distro_release="Other"
    fi
fi

arg_username=""
arg_privacymode=
arg_category="all"              # if no arguments, just execute rules in category_all()
arg_outputfile=
arg_maxsize=5242880             # 5MB
arg_sliceinfo_type=
arg_complete_opt=
arg_distro_name=
arg_distro_release=
arg_help=
apostrophe="'"

show_usage() {
    cat <<EOF
${app_name} [-u <username>] [-p] [-d <sliceinfo>] [-o <filename>] [-m <maxsize>] [-h] [<category>]
Options:
    -u, --username, collect information for target user.
    -p, --privacy-mode, enable privacy mode
    -d, --dump, print system slice information, the type coulde be:
$(get_sliceinfo_funcs)
    -o, --output, customize the output file
    -m, --maxsize, set single archive file${apostrophe}s maximize size
    --distro-name, print current distributor name
    --distro-release, print current distributor release version
    -h, --help, show this message

    If there is no other arguments, ${app_name} will collect debug
    information for special category and save it to archive file in
    current directory, the category could be (default: all):
$(get_category_funcs)
EOF
}

# dispatch arguments
while [ ${#} -gt 0 ]; do
    case ${1} in
        -u|--username) arg_username="${2}"; shift; shift;;
        -p|--privacy-mode) arg_privacymode=t; shift;;
        -d|--dump) arg_sliceinfo_type="${2}"; arg_category=""; shift; shift; break;;
        -o|--output) arg_outputfile="${2}"; shift; shift;;
        -m|--maxsize) arg_maxsize="${2}"; shift; shift;;
        --distro-name) arg_distro_name=t; break;;
        --distro-release) arg_distro_release=t; break;;
        -h|--help) arg_help=t; break;;
        -C|--complete) arg_complete_opt="${2}"; shift; shift; break;;
        *)  arg_category="${1}"; shift;;
    esac
done

if [ "${arg_help}" ]; then
    show_usage
    exit 1
fi

if [ "${arg_distro_name}" ]; then
    printf "${distro_name}"
    exit
fi

if [ "${arg_distro_release}" ]; then
    printf "${distro_release}"
    exit
fi

if [ "${arg_username}" ]; then
    real_home="/home/${arg_username}"
fi

if [ "${arg_complete_opt}" ]; then
    case "${arg_complete_opt}" in
        -p) get_sliceinfo_funcs;;
        *)  get_category_funcs;;
    esac
    exit
fi

if [ "${arg_sliceinfo_type}" ]; then
    case "${arg_sliceinfo_type}" in
        syslog)
            for a in ${@}; do
                include_syslog_keyword "${a}"
            done;;
    esac
    sliceinfo_"${arg_sliceinfo_type}"
    exit
fi

if [ "${arg_category}" ]; then
    # ensure this script running as root
    if [ ! "${UID}" -eq 0 ]; then
        printf "please run ${app_name} as root, just use: sudo -E ${app_name}"
        exit 1
    fi

    # global variables
    result_tag="${app_name}-${distro_name}-${distro_release}-${arg_category}-$(date "+%Y%m%d-%H%M%S")"
    dest_dir="/tmp/${result_tag}"
    result_archive="${arg_outputfile:-${result_tag}.tar.gz}"
    file_master="${dest_dir}/sysinfo.md"
    file_syslog="${dest_dir}/syslog"
    file_service="${dest_dir}/services.md"
    file_env="${dest_dir}/env"
    file_package="${dest_dir}/packages.md"
    file_bootmgr="${dest_dir}/bootmgr.md"
    file_device="${dest_dir}/devices.md"
    file_driver="${dest_dir}/drivers.md"
    file_fonts="${dest_dir}/fonts"
    file_kernel="${dest_dir}/dmesg"
    file_gsettings="${dest_dir}/gsettings.md"
    file_fprint="${dest_dir}/fprint.md"

    msg "Collecting system information and this will take several seconds..."

    # prepare
    msg "Preparing temporary folder ${dest_dir}..."
    rm -rf "${dest_dir}"
    mkdir -p "${dest_dir}"

    # execute rules of category
    msg "Execute rules of category '${arg_category}'..."
    if ! category_"${arg_category}"; then
        exit 1
    fi
    exec_sliceinfo_funcs

    # archive files
    msg "Archive files..."
    dest_files=$(ls -1 "${dest_dir}" | tr "\n" " ")
    tar -cvzf "${result_archive}" -h -C "${dest_dir}" ${dest_files}
    rm -rf ${dest_dir}

    # split archive file if need
    archive_size="$(stat -c "%s" "${result_archive}")"
    if [ "${archive_size}" -ge "${arg_maxsize}" ]; then
        msg "Split archive file..."
        rm -vf "${result_archive}".part.*
        split -b "${arg_maxsize}" -d "${result_archive}" "${result_archive}".part.
        rm -f "${result_archive}"
    fi

    export TEXTDOMAIN=deepin-feedback
    uploadPrompt=$(gettext "Please report the bug by uploading %s as an attachment to deepin feedback: %s, so that deepin team can locate and solve the problem.")
    uploadPrompt=$(printf "$uploadPrompt\n" $(pwd)/$result_archive "http://feedback.deepin.org/feedback")
    msg "$uploadPrompt"

    exit
fi
