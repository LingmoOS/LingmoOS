#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.

exec 3>&1

Echo ()
{
	local STRING="${1}"
	shift

	printf "${STRING}\n" "${@}" >&3
}

Echo_debug ()
{
	if [ "${_DEBUG}" = "true" ]; then
		local STRING="${1}"
		shift

		printf "D: ${STRING}\n" "${@}" >&3
	fi
}

Echo_error ()
{
	local STRING="${1}"
	shift

	local PREFIX="${RED}E${NO_COLOR}"
	if [ "${_COLOR_ERR}" = "false" ]; then
		PREFIX="E"
	fi

	printf "${PREFIX}: ${STRING}\n" "${@}" >&2
}

Echo_message ()
{
	if [ "${_QUIET}" != "true" ]
	then
		local STRING="${1}"
		shift

		local PREFIX="${PURPLE}P${NO_COLOR}"
		if [ "${_COLOR_OUT}" = "false" ]; then
			PREFIX="P"
		fi

		printf "${PREFIX}: ${STRING}\n" "${@}" >&3
	fi
}

Echo_verbose ()
{
	if [ "${_VERBOSE}" = "true" ]; then
		local STRING="${1}"
		shift

		printf "I: ${STRING}\n" "${@}" >&3
	fi
}

Echo_warning ()
{
	local STRING="${1}"
	shift

	local PREFIX="${YELLOW}W${NO_COLOR}"
	if [ "${_COLOR_ERR}" = "false" ]; then
		PREFIX="W"
	fi

	printf "${PREFIX}: ${STRING}\n" "${@}" >&2
}

Echo_file ()
{
	local LINE
	while read -r LINE
	do
		echo "${1}: ${LINE}" >&3
	done < "${1}"
}
