#!/bin/sh

## lingmo-system-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


# The file that records temporarily installed packages.
Installed_tmp_packages_file ()
{
	echo "chroot.installed_tmp_pkgs"
}

# Note, writes to _LINGMO_SYS_BUILD_PACKAGES
Check_package ()
{
	local CHROOT="${1}"
	local FILE="${2}"
	local PACKAGE="${3}"

	Check_installed "${CHROOT}" "${FILE}" "${PACKAGE}"

	if [ "${INSTALL_STATUS}" -ne 0 ]
	then
		if [ "${LINGMO_SYS_BUILD_BUILD_WITH_CHROOT}" != "false" ] && [ "${CHROOT}" = "chroot" ]
		then
			_LINGMO_SYS_BUILD_PACKAGES="${_LINGMO_SYS_BUILD_PACKAGES} ${PACKAGE}"
		else
			Echo_error "You need to install %s on your host system." "${PACKAGE}"
			exit 1
		fi
	fi
}

# Note, reads from _LINGMO_SYS_BUILD_PACKAGES
Install_packages ()
{
	if [ -z "${_LINGMO_SYS_BUILD_PACKAGES}" ] || [ "${LINGMO_SYS_BUILD_BUILD_WITH_CHROOT}" != "true" ]; then
		return
	fi

	# Record in file to survive failure such that recovery can take place.
	local LIST_FILE
	LIST_FILE="$(Installed_tmp_packages_file)"
	local PACKAGE
	for PACKAGE in ${_LINGMO_SYS_BUILD_PACKAGES}; do
		echo "${PACKAGE}" >> "${LIST_FILE}"
	done

	case "${LINGMO_SYS_BUILD_APT}" in
		apt|apt-get)
			Chroot chroot "apt-get install -o APT::Install-Recommends=false ${APT_OPTIONS} ${_LINGMO_SYS_BUILD_PACKAGES}"
			;;

		aptitude)
			Chroot chroot "aptitude install --without-recommends ${APTITUDE_OPTIONS} ${_LINGMO_SYS_BUILD_PACKAGES}"
			;;
	esac
	unset _LINGMO_SYS_BUILD_PACKAGES # Can clear this now
}

Remove_packages ()
{
	if [ "${LINGMO_SYS_BUILD_BUILD_WITH_CHROOT}" != "true" ]; then
		return
	fi

	local LIST_FILE
	LIST_FILE="$(Installed_tmp_packages_file)"

	# List is read from file to ensure packages from any past failure are
	# included in the list on re-running scripts to recover.
	local PACKAGES=""
	if [ -e "${LIST_FILE}" ]; then
		local PACKAGE
		while read -r PACKAGE; do
			PACKAGES="${PACKAGES} ${PACKAGE}"
		done < "${LIST_FILE}"
	fi

	if [ -n "${PACKAGES}" ]; then
		case "${LINGMO_SYS_BUILD_APT}" in
			apt|apt-get)
				Chroot chroot "apt-get remove --auto-remove --purge ${APT_OPTIONS} ${PACKAGES}"
				;;

			aptitude)
				Chroot chroot "aptitude purge --purge-unused ${APTITUDE_OPTIONS} ${PACKAGES}"
				;;
		esac
	fi

	rm -f "${LIST_FILE}"
}

#FIXME: make use of this. see commit log that added this for details.
# Perform temp package removal for recovery if necessary
Cleanup_temp_packages ()
{
	if [ -e "$(Installed_tmp_packages_file)" ]; then
		Remove_packages
	fi
}

# Check_installed
# uses as return value global var INSTALL_STATUS
# INSTALL_STATUS : 0 if package is installed
#                  1 if package isn't installed and we're in an apt managed system
#                  2 if package isn't installed and we aren't in an apt managed system
Check_installed ()
{
	local CHROOT="${1}"
	local FILE="${2}"
	local PACKAGE="${3}"

	if [ "${LINGMO_SYS_BUILD_BUILD_WITH_CHROOT}" = "true" ] && [ "${CHROOT}" = "chroot" ]
	then
		if Chroot chroot "dpkg-query -s ${PACKAGE}" 2> /dev/null | grep -qs "Status: install"
		then
			INSTALL_STATUS=0
		else
			INSTALL_STATUS=1
		fi
	else
		if [ -e /etc/debian_version ]
		then
			if dpkg-query -s "${PACKAGE}" 2> /dev/null | grep -qs "Status: install"
			then
				INSTALL_STATUS=0
			else
				INSTALL_STATUS=1
			fi
		else
			if [ ! -e "${FILE}" ]
			then
				INSTALL_STATUS=2
			else
				INSTALL_STATUS=0
			fi
		fi
	fi
}

