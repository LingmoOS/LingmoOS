#!/bin/sh

## lingmo-system-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Create_apt_sources_list ()
{
	local PARENT_MIRROR
	local MIRROR
	local PARENT_MIRROR_SECURITY
	local MIRROR_SECURITY
	local PARENT_DISTRIBUTION
	local DISTRIBUTION

	case "${1}" in
		chroot)
			PARENT_MIRROR=${LINGMO_SYS_BUILD_PARENT_MIRROR_CHROOT}
			MIRROR=${LINGMO_SYS_BUILD_MIRROR_CHROOT}
			PARENT_MIRROR_SECURITY=${LINGMO_SYS_BUILD_PARENT_MIRROR_CHROOT_SECURITY}
			MIRROR_SECURITY=${LINGMO_SYS_BUILD_MIRROR_CHROOT_SECURITY}
			PARENT_DISTRIBUTION=${LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_CHROOT}
			DISTRIBUTION=${LINGMO_SYS_BUILD_DISTRIBUTION_CHROOT}
			;;
		binary)
			PARENT_MIRROR=${LINGMO_SYS_BUILD_PARENT_MIRROR_BINARY}
			MIRROR=${LINGMO_SYS_BUILD_MIRROR_BINARY}
			PARENT_MIRROR_SECURITY=${LINGMO_SYS_BUILD_PARENT_MIRROR_BINARY_SECURITY}
			MIRROR_SECURITY=${LINGMO_SYS_BUILD_MIRROR_BINARY_SECURITY}
			PARENT_DISTRIBUTION=${LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_BINARY}
			DISTRIBUTION=${LINGMO_SYS_BUILD_DISTRIBUTION_BINARY}
			;;
		*)
			Echo_error "Invalid mode '${1}' specified for source list creation!"
			exit 1
			;;
	esac

	local _PASS="${2}"

	local PARENT_FILE
	case "${LINGMO_SYS_BUILD_DERIVATIVE}" in
		true)
			PARENT_FILE="sources.list.d/debian.list"
			;;

		false)
			PARENT_FILE="sources.list"
			;;
	esac

	local LIST_FILE="chroot/etc/apt/sources.list.d/${LINGMO_SYS_BUILD_MODE}.list"
	local PARENT_LIST_FILE="chroot/etc/apt/${PARENT_FILE}"

	local _DISTRIBUTION
	if [ "${LINGMO_SYS_BUILD_DERIVATIVE}" = "true" ]; then
		_DISTRIBUTION="$(echo ${DISTRIBUTION} | sed -e 's|-backports||')"
	fi

	# Clear out existing lists
	rm -f ${PARENT_LIST_FILE} ${LIST_FILE}

	# Set general repo
	echo "deb ${PARENT_MIRROR} ${PARENT_DISTRIBUTION} ${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS}" >> ${PARENT_LIST_FILE}
	echo "deb-src ${PARENT_MIRROR} ${PARENT_DISTRIBUTION} ${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS}" >> "${PARENT_LIST_FILE}"

	if [ "${LINGMO_SYS_BUILD_DERIVATIVE}" = "true" ]; then
		echo "deb ${MIRROR} ${_DISTRIBUTION} ${LINGMO_SYS_BUILD_ARCHIVE_AREAS}" >> "${LIST_FILE}"
		echo "deb-src ${MIRROR} ${_DISTRIBUTION} ${LINGMO_SYS_BUILD_ARCHIVE_AREAS}" >> "${LIST_FILE}"
	fi

	# Set security repo
	if [ "${LINGMO_SYS_BUILD_SECURITY}" = "true" ]; then
		case "${LINGMO_SYS_BUILD_MODE}" in
			debian)
				case "${PARENT_DISTRIBUTION}" in
					sid|unstable)
						# do nothing
						;;

					buster|jessie|stretch)
						echo "deb ${PARENT_MIRROR_SECURITY} ${PARENT_DISTRIBUTION}/updates ${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS}" >> "${PARENT_LIST_FILE}"
						echo "deb-src ${PARENT_MIRROR_SECURITY} ${PARENT_DISTRIBUTION}/updates ${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS}" >> "${PARENT_LIST_FILE}"
						;;
					*)
						echo "deb ${PARENT_MIRROR_SECURITY} ${PARENT_DISTRIBUTION}-security ${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS}" >> "${PARENT_LIST_FILE}"
						echo "deb-src ${PARENT_MIRROR_SECURITY} ${PARENT_DISTRIBUTION}-security ${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS}" >> "${PARENT_LIST_FILE}"
						;;
				esac

				if [ "${LINGMO_SYS_BUILD_DERIVATIVE}" = "true" ]; then
					echo "deb ${MIRROR_SECURITY} ${_DISTRIBUTION}/updates ${LINGMO_SYS_BUILD_ARCHIVE_AREAS}" >> "${LIST_FILE}"
					echo "deb-src ${MIRROR_SECURITY} ${_DISTRIBUTION}/updates ${LINGMO_SYS_BUILD_ARCHIVE_AREAS}" >> "${LIST_FILE}"
				fi
				;;
		esac
	fi

	# Set updates repo
	if [ "${LINGMO_SYS_BUILD_UPDATES}" = "true" ]; then
		echo "deb ${PARENT_MIRROR} ${PARENT_DISTRIBUTION}-updates ${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS}" >> "${PARENT_LIST_FILE}"
		echo "deb-src ${PARENT_MIRROR} ${PARENT_DISTRIBUTION}-updates ${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS}" >> "${PARENT_LIST_FILE}"

		if [ "${LINGMO_SYS_BUILD_DERIVATIVE}" = "true" ]; then
			echo "deb ${MIRROR} ${_DISTRIBUTION}-updates ${LINGMO_SYS_BUILD_ARCHIVE_AREAS}" >> "${LIST_FILE}"
			echo "deb-src ${MIRROR} ${_DISTRIBUTION}-updates ${LINGMO_SYS_BUILD_ARCHIVE_AREAS}" >> "${LIST_FILE}"
		fi
	fi

	# Set backports repo
	if [ "${LINGMO_SYS_BUILD_BACKPORTS}" = "true" ]; then
		case "${LINGMO_SYS_BUILD_MODE}" in
			debian)
				if [ "${PARENT_DISTRIBUTION}" != "sid" ] && [ "${PARENT_DISTRIBUTION}" != "unstable" ]; then
					echo "deb ${PARENT_MIRROR} ${PARENT_DISTRIBUTION}-backports ${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS}" >> "${PARENT_LIST_FILE}"
					echo "deb-src ${PARENT_MIRROR} ${PARENT_DISTRIBUTION}-backports ${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS}" >> "${PARENT_LIST_FILE}"
				fi
				;;
		esac

		if [ "${LINGMO_SYS_BUILD_DERIVATIVE}" = "true" ]; then
			echo "deb ${MIRROR} ${_DISTRIBUTION}-backports ${LINGMO_SYS_BUILD_ARCHIVE_AREAS}" >> "${LIST_FILE}"
			echo "deb-src ${MIRROR} ${_DISTRIBUTION}-backports ${LINGMO_SYS_BUILD_ARCHIVE_AREAS}" >> "${LIST_FILE}"
		fi
	fi

	# Disable deb-src entries?
	if [ "${_PASS}" != "source" ] && [ "${LINGMO_SYS_BUILD_APT_SOURCE_ARCHIVES}" != "true" ]; then
		sed -i "s/^deb-src/#deb-src/g" "${PARENT_LIST_FILE}"
		if [ "${LINGMO_SYS_BUILD_DERIVATIVE}" = "true" ]; then
			sed -i "s/^deb-src/#deb-src/g" "${LIST_FILE}"
		fi
	fi
}
