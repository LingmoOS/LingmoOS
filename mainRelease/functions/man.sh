#!/bin/sh

## lingmo-system-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


Man ()
{
	if command -v man >/dev/null; then
		man ${PROGRAM}
	else
		Echo_warning "man is not installed, falling back to usage output."
		Usage
	fi
	exit 0
}
