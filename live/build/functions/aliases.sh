#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Find_files ()
{
	(ls "${@}" | grep -qs .) > /dev/null 2>&1
}

In_list ()
{
	local NEEDLE="${1}"
	shift

	local ITEM
	for ITEM in "${@}"; do
		if [ "${NEEDLE}" = "${ITEM}" ]; then
			return 0
		fi
	done

	return 1
}

Truncate ()
{
	local FILE
	for FILE in "${@}"; do
		if [ ! -L ${FILE} ]
		then
			: > ${FILE}
		fi
	done
}
