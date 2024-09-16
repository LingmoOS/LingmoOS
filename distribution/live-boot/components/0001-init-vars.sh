#!/bin/sh

mountpoint="/run/live/medium"
alt_mountpoint="/media"
LIVE_MEDIA_PATH="live"
HOSTNAME="host"
custom_overlay_label="persistence"
persistence_list="persistence.conf"

mkdir -p "${mountpoint}"
mkdir -p /var/lib/live/boot
