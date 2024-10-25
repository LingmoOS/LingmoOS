#!/bin/sh

set -e

rootdir=$1

# common needs rootdir to already be defined.
. /w/in/common

trap cleanup 0

mount_support
disable_daemons
prepare_apt_source

chroot ${rootdir} apt-get -q -y install ${TASK_PACKAGES} ${EXTRA_PACKAGES} \
 task-xfce-desktop exim4 mutt info rpcbind pciutils \
 task-ssh-server task-print-server plymouth procmail \
 m4 open-vm-tools apt-listchanges at busybox nfs-common \
 wamerican texinfo plymouth-themes plymouth-x11 uuid-runtime \
 open-vm-tools-dkms open-vm-tools-desktop gettext-base mlocate \
 irqbalance memtest86+ user-setup zerofree

remove_daemon_block
replace_apt_source

echo "Customisation complete"
