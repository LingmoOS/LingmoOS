#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Breakpoint ()
{
	local NAME="${1}"

	if [ "${_BREAKPOINTS}" = "true" ]
	then
		Echo_message "Waiting at %s" "${NAME}"
		read WAIT
	fi
}
