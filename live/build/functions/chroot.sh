#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Chroot ()
{
	local CHROOT="${1}"; shift
	local COMMANDS
	COMMANDS="${@}" #must be on separate line to 'local' declaration to avoid error

	# Executing commands in chroot
	Echo_debug "Executing: %s" "${COMMANDS}"

	local ENV=""

	local _FILE
	for _FILE in config/environment config/environment.chroot
	do
		if [ -e "${_FILE}" ]
		then
			ENV="${ENV} $(grep -v '^#' ${_FILE})"
		fi
	done

	${_LINUX32} chroot "${CHROOT}" /usr/bin/env -i HOME="/root" PATH="/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin" TERM="${TERM}" DEBIAN_FRONTEND="${LB_DEBCONF_FRONTEND}" DEBIAN_PRIORITY="${LB_DEBCONF_PRIORITY}" DEBCONF_NONINTERACTIVE_SEEN="true" DEBCONF_NOWARNINGS="true" SOURCE_DATE_EPOCH=${SOURCE_DATE_EPOCH} http_proxy=${http_proxy} no_proxy=${no_proxy} ${ENV} ${COMMANDS}

	return ${?}
}

Chroot_has_package() {
	local PACKAGE="${1}"; shift
	local CHROOT="${2:-chroot}"; shift

	if dpkg-query --admindir=${CHROOT}/var/lib/dpkg -s ${PACKAGE} >/dev/null 2>&1 | grep -q "^Status: install"
	then
		return 0
	fi
	return 1
}

Chroot_package_list() {
	local CHROOT="${1:-chroot}"; shift

	dpkg-query --admindir=${CHROOT}/var/lib/dpkg -W -f'${Package}\n'
}

Chroot_copy_dir() {
	local DIR="${1}"
	local NAME="${2:-$(basename ${DIR})}"

	Check_installed host /usr/bin/rsync rsync
	if [ "${INSTALL_STATUS}" -eq "0" ]
	then
		Echo_message "Copying ${NAME} into chroot using rsync..."
		rsync -Klrv --chown=0:0 "${DIR}" chroot/
	else
		cd "${DIR}"
		Echo_message "Creating a tarball with files from ${NAME}..."
		tar cf "${OLDPWD}"/chroot/"${NAME}".tar .
		cd "${OLDPWD}"
		Echo_message "Extracting the tarball in the chroot..."
		Chroot chroot "tar -xvf ${NAME}.tar --no-same-owner --keep-directory-symlink --overwrite"
		rm chroot/"${NAME}".tar
	fi
}
