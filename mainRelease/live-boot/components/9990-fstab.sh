#!/bin/sh

#set -e

Fstab ()
{
	if [ -n "${NOFSTAB}" ]
	then
		return
	fi

	log_begin_msg "Configuring fstab"

	if ! grep -qs  "^${UNIONTYPE}" /root/etc/fstab
	then
		echo "${UNIONTYPE} / ${UNIONTYPE} rw 0 0" >> /root/etc/fstab
	fi

	if ! grep -qs "^tmpfs /tmp" /root/etc/fstab
	then
		echo "tmpfs /tmp tmpfs nosuid,nodev 0 0" >> /root/etc/fstab
	fi

	log_end_msg
}
