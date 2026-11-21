#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Acquire_lockfile ()
{
	local FILE="${1:-.lock}"
	Check_lockfile "${FILE}"
	Create_lockfile "${FILE}"
}

Check_lockfile ()
{
	local FILE="${1}"

	if [ -f "${FILE}" ]; then
		Echo_error "${PROGRAM} already locked"
		exit 1
	fi
}

Create_lockfile ()
{
	local FILE="${1}"

	# Create lock trap
	# This automatically removes the lock file in certain conditions
	trap 'ret=${?}; '"rm -f \"${FILE}\";"' exit ${ret}' EXIT HUP INT QUIT TERM

	# Creating lock file
	touch "${FILE}"
}

Remove_lockfile ()
{
	local FILE="${1:-.lock}"

	rm -f "${FILE}"
}
