#!/bin/sh

# Defaults
LIVE_HOSTNAME="debian"
LIVE_USERNAME="user"
LIVE_USER_FULLNAME="Debian Live user"
LIVE_USER_DEFAULT_GROUPS="audio cdrom dip floppy video plugdev netdev powerdev scanner bluetooth debian-tor"
export LIVE_HOSTNAME LIVE_USERNAME LIVE_USER_FULLNAME LIVE_USER_DEFAULT_GROUPS

# Reading configuration files from filesystem and live-media
set -o allexport
for _FILE in /etc/live/config.conf /etc/live/config.conf.d/*.conf \
	     /lib/live/mount/medium/live/config.conf /lib/live/mount/medium/live/config.conf.d/*.conf \
	     /lib/live/mount/persistence/*/live/config.conf /lib/live/mount/persistence/*/live/config.conf.d/*.conf
do
	if [ -e "${_FILE}" ]
	then
		. "${_FILE}"
	fi
done
set +o allexport
