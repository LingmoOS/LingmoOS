#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Usage ()
{
	echo "${PROGRAM_NAME} - ${DESCRIPTION}"
	printf "\nUsage:\n\n"

	if [ -n "${USAGE}" ]; then
		# printf without placeholder required here for correct \t and \n formatting of `lb config` usage string
		printf "  ${USAGE}\n"
	fi

	echo "  ${PROGRAM} [-h|--help]"
	echo "  ${PROGRAM} [-u|--usage]"
	echo "  ${PROGRAM} [-v|--version]"
	echo
	echo "Try \"${PROGRAM} --help\" for more information."

	case $1 in
		--fail)
			exit 1
			;;
		--exit)
			exit 0
			;;
		"")
			:
			;;
		*)
			Echo_error "Unexpected parameter to Usage(): $1"
			;;
	esac
}
