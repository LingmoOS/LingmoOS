#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Restore_package_cache ()
{
	local DIRECTORY="cache/packages.${1}"

	if [ "${LB_CACHE}" = "true" ] && [ "${LB_CACHE_PACKAGES}" = "true" ]
	then
		if [ -e "${DIRECTORY}" ]
		then
			# Restore old cache
			if [ "$(stat --printf %d ${DIRECTORY}/)" = "$(stat --printf %d chroot/var/cache/apt/archives/)" ]
			then
				# with hardlinks
				find "${DIRECTORY}" -name "*.deb" -print0 | xargs -0 --no-run-if-empty cp -fl -t chroot/var/cache/apt/archives
			else
				# without hardlinks
				find "${DIRECTORY}" -name "*.deb" -print0 | xargs -0 --no-run-if-empty cp -t chroot/var/cache/apt/archives
			fi
		fi
	fi
}

Save_package_cache ()
{
	local DIRECTORY="cache/packages.${1}"

	if [ "${LB_CACHE}" = "true" ] && [ "${LB_CACHE_PACKAGES}" = "true" ]
	then
		# Cleaning current cache
		# In case of interrupted or incomplete builds, this may return an error,
		# but we still do want to save the cache.
		Chroot chroot "apt-get autoclean" || true

		if ls chroot/var/cache/apt/archives/*.deb > /dev/null 2>&1
		then
			# Creating cache directory
			mkdir -p "${DIRECTORY}"

			# Saving new cache
			local PACKAGE
			for PACKAGE in chroot/var/cache/apt/archives/*.deb
			do
				if [ -e "${DIRECTORY}"/"$(basename ${PACKAGE})" ]
				then
					rm -f "${PACKAGE}"
				else
					mv "${PACKAGE}" "${DIRECTORY}"
				fi
			done
		fi
	else
		# Purging current cache
		rm -f chroot/var/cache/apt/archives/*.deb
	fi
}
