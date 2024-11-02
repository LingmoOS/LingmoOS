#!/bin/bash

set -e

. /tmp/utils

#timedatectl set-ntp true
mkdir -p /var/log/installer

do_inchroot() {
  # 运行 "${IN_TARGET}"/in_chroot/ 脚本
  run_scripts_dir "${IN_TARGET}"/in_chroot

  if [[ -d /cdrom/hooks/in_chroot ]]; then
    run_scripts_dir /cdrom/hooks/in_chroot
  fi
}

do_inchroot

msg "k.sh 运行结束"
