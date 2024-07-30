#!/bin/sh

# SPDX-License-Identifier: GPL-3.0-or-later
# SPDX-FileCopyrightText: 2021 Aditya Mehra <aix.m@outlook.com>

OPTS=$(getopt -o '' --long enable,disable -- "$@")

if [ ! $? ]; then echo "Failed parsing options." >&2; exit 1; fi

eval set -- "$OPTS"

ENABLE=false
while true; do
    case "$1" in
        --enable) ENABLE=true;;
        --disable) ENABLE=false;;
		--) shift; break ;;
	esac
	shift
done
shift $((OPTIND-1))

rm /etc/xdg/bigscreen

if $ENABLE; then
	printf "[General]\nMycroftEnabled=true\n" >> /etc/xdg/bigscreen
else
	printf "[General]\nMycroftEnabled=false\n" >> /etc/xdg/bigscreen
fi

if [ -f /opt/mycroft ]; then
	SCRIPTFILE=/etc/pico-wizard/scripts.d/01-fix-mycroft-perms

	if [ -f "$SCRIPTFILE" ]; then
	    echo "$SCRIPTFILE exist skipping"
	else
	    mkdir -p /etc/pico-wizard/scripts.d/
	    printf "#! /bin/bash\n\nchown -R \$PICOWIZARD_USERNAME:\$PICOWIZARD_USERNAME /opt/mycroft-core/\nchown -R \$PICOWIZARD_USERNAME:\$PICOWIZARD_USERNAME /opt/mycroft/" >> /etc/pico-wizard/scripts.d/01-fix-mycroft-perms
	    chmod 755 /etc/pico-wizard/scripts.d/01-fix-mycroft-perms
	fi
fi
