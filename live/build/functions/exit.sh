#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Exit ()
{
	local VALUE=$1

	if [ "${_DEBUG}" = "true" ]
	then
		# Dump variables
		set | grep -e ^LB
	fi

	# Skip if we have not yet completed the initial bootstrapping (bootstrap_debootstrap)
	# (nothing to be done; avoids unhelpful messages)
	if ! Stagefile_exists bootstrap; then
		return ${VALUE}
	fi

	# Always exit true in case we are not able to unmount
	# (e.g. due to running processes in chroot from user customizations)
	Echo_message "Begin unmounting filesystems..."

	local DIRECTORY
	if [ -e /proc/mounts ]
	then
		for DIRECTORY in $(awk -v dir="${PWD}/chroot/" '$2 ~ dir { print $2 }' /proc/mounts | sort -r)
		do
			umount ${DIRECTORY} > /dev/null 2>&1 || true
		done
	else
		for DIRECTORY in /dev/shm /dev/pts /dev /proc /sys/fs/selinux /sys /root/config
		do
			umount -f chroot/${DIRECTORY} > /dev/null 2>&1 || true
		done
	fi

	STAGEFILES_DIR="$(Stagefiles_dir)"

	rm -f "${STAGEFILES_DIR}"/chroot_devpts
	rm -f "${STAGEFILES_DIR}"/chroot_proc
	rm -f "${STAGEFILES_DIR}"/chroot_selinuxfs
	rm -f "${STAGEFILES_DIR}"/chroot_sysfs

	Echo_message "Saving caches..."

	# We can't really know at which part we're failing,
	# but let's assume that if there's any binary stage file arround
	# we are in binary stage.

	if ls "${STAGEFILES_DIR}"/binary* > /dev/null 2>&1
	then
		Save_package_cache binary
	else
		Save_package_cache chroot
	fi

	return ${VALUE}
}

Exit_exit ()
{
	local VALUE=$?
	if [ "${VALUE}" -ne 0 ]; then
		Echo_error "An unexpected failure occurred, exiting..."
	fi
	Exit ${VALUE}
}

Exit_other ()
{
	local VALUE=$?
	Echo_warning "Unexpected early exit caught, attempting cleanup..."
	Exit ${VALUE}
}

Setup_clean_exit ()
{
	Echo_message "Setting up clean exit handler"
	trap 'Exit_other' HUP INT QUIT TERM
	trap 'Exit_exit' EXIT
}
