#!/bin/sh

## lingmo-system-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Common_conffiles ()
{
	echo "config/all config/common config/bootstrap config/chroot config/binary config/source"
}

Get_conffiles ()
{
	local FILES
	if [ -n "${LINGMO_SYS_BUILD_CONFIG}" ]
	then
		FILES="${LINGMO_SYS_BUILD_CONFIG}"
	else
		# List standard files first, then possible user arch/dist overrides
		FILES="${@}"
		local FILE
		for FILE in "${@}"; do
			FILES="${FILES} ${LINGMO_SYS_BUILD_ARCHITECTURE:+$FILE.$LINGMO_SYS_BUILD_ARCHITECTURE}"
			FILES="${FILES} ${LINGMO_SYS_BUILD_DISTRIBUTION:+$FILE.$LINGMO_SYS_BUILD_DISTRIBUTION}"
		done
	fi

	echo ${FILES}
}

Read_conffiles ()
{
	local CONFFILE
	for CONFFILE in $(Get_conffiles "${@}")
	do
		if [ -f "${CONFFILE}" ]
		then
			if [ -r "${CONFFILE}" ]
			then
				Echo_debug "Reading configuration file %s" "${CONFFILE}"
				. "${CONFFILE}"
			else
				Echo_warning "Failed to read configuration file %s" "${CONFFILE}"
			fi
		fi
	done
}

Print_conffiles ()
{
	local CONFFILE
	for CONFFILE in $(Get_conffiles "${@}")
	do
		if [ -f "${CONFFILE}" ]
		then
			Echo_file "${CONFFILE}"
		fi
	done
}
