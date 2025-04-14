#!/bin/sh

CHROOT=$(mount | grep proc | grep calamares | awk '{print $3}' | sed -e "s#/proc##g")

if [ "$1" = "-u" ]; then
    rm $CHROOT/etc/dpkg/dpkg.cfg.d/calamares-force-unsafe-io
    sync
    exit 0
fi

echo "force-unsafe-io" > $CHROOT/etc/dpkg/dpkg.cfg.d/calamares-force-unsafe-io

exit 0
