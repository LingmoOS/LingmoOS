#!/bin/sh

## lingmo-system-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Chroot_bind_path ()
{
	local CHROOT
	local BIND_SRC
	local BIND_DEST
	CHROOT="$(readlink -f ${1})"
	BIND_SRC="$(readlink -f ${2})"

	BIND_DEST=$(echo "${BIND_SRC}" | sed -e 's|/\+||')
	if [ ! -d "${CHROOT}/${BIND_DEST}" -o \
		-z "$(cat /proc/mounts | awk -vdir="${CHROOT}/${BIND_DEST}" '$2 ~ dir { print $2}')" ]
	then
		Echo_message "Binding local repository path"
		mkdir -p "${CHROOT}/${BIND_DEST}"
		mount --bind "${LINGMO_SYS_BUILD_PARENT_MIRROR_CHROOT#file:}" \
			"${CHROOT}/${BIND_DEST}"
	fi
}

Chroot_unbind_path ()
{
	local CHROOT
	local BIND_SRC
	local BIND_DEST
	CHROOT="$(readlink -f ${1})"
	BIND_SRC="$(readlink -f ${2})"

	BIND_DEST=$(echo "${BIND_SRC}" | sed -e 's|/\+||')
	if [ -d "${CHROOT}/${BIND_DEST}" ]
	then
		Echo_message "Unbinding local repository path"
		umount "${CHROOT}/${BIND_DEST}"  > /dev/null 2>&1 || true
	fi
}
