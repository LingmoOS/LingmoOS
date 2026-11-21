#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2025 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


# Try to bind a file URI from the host into the chroot
# - If the URI does not use the scheme 'file' -> does nothing
# - If the URI does not point to an absolute path -> does nothing
#
# $1 = directory of the chroot
# $2 = URI of scheme file (file://localhost/absolute-path or file:///absolute-path)
Chroot_try_bind_path ()
{
	local CHROOT
	local BIND_SRC
	local BIND_DEST
	CHROOT="$(readlink -f ${1})"
	if echo "${2}" | grep -E -q '^file://(localhost)?/'
	then
		BIND_SRC="$(readlink -f $(echo ${2} | sed --regexp-extended -e 's|^file://(localhost)?||'))"

		BIND_DEST=${CHROOT}${BIND_SRC}
		if [ ! -d "${BIND_DEST}" -o \
			-z "$(cat /proc/mounts | awk -vdir="${BIND_DEST}" '$2 ~ dir { print $2}')" ]
		then
			Echo_message "Binding local repository path ${BIND_SRC}"
			mkdir -p "${BIND_DEST}"
			mount --bind "${BIND_SRC}" "${BIND_DEST}"
		fi
	fi
}

Chroot_try_unbind_path ()
{
	local CHROOT
	local BIND_SRC
	local BIND_DEST
	CHROOT="$(readlink -f ${1})"
	if echo "${2}" | grep -E -q '^file://(localhost)?/'
	then
		BIND_SRC="$(readlink -f $(echo ${2} | sed --regexp-extended -e 's|^file://(localhost)?||'))"

		BIND_DEST=${CHROOT}${BIND_SRC}
		if [ -d "${BIND_DEST}" ]
		then
			Echo_message "Unbinding local repository path"
			umount "${BIND_DEST}"  > /dev/null 2>&1 || true
			rmdir --parents "${BIND_DEST}" || true
		fi
	fi
}

# Try to bind the first URI from a sources.list(5) file in one-line-style format
#
# $1 = directory of the chroot
# $2 = file in sources.list(5) one-line-style format
Chroot_try_bind_path_from_list ()
{
	local CHROOT
	local URI
	local FILE
	CHROOT=${1}
	FILE=${2}
	URI=$(sed --regexp-extended -e 's/.*(file:\/\/[^ ]+).*/\1/' ${FILE} | head -1)
	Chroot_try_bind_path ${CHROOT} ${URI}
}

Chroot_try_unbind_path_from_list ()
{
	local CHROOT
	local URI
	local FILE
	CHROOT=${1}
	FILE=${2}
	URI=$(sed --regexp-extended -e 's/.*(file:\/\/[^ ]+).*/\1/' ${FILE} | head -1)
	Chroot_try_unbind_path ${CHROOT} ${URI}
}

