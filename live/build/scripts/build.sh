#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program. If not, see <http://www.gnu.org/licenses/>.
##
## The complete text of the GNU General Public License
## can be found in /usr/share/common-licenses/GPL-3 file.


set -e

if [ -z "${_COLOR}" ]; then
	_COLOR="auto"
	_COLOR_OUT="true"
	_COLOR_ERR="true"
	if [ ! -t 1 ]; then
		_COLOR_OUT="false"
	fi
	if [ ! -t 2 ]; then
		_COLOR_ERR="false"
	fi
else
	_COLOR_OUT="${_COLOR}"
	_COLOR_ERR="${_COLOR}"
fi

if [ -e local/live-build ]
then
	LIVE_BUILD="${LIVE_BUILD:-${PWD}/local/live-build}"
	export LIVE_BUILD
fi

for _DIRECTORY in "${LIVE_BUILD}/functions" /usr/share/live/build/functions
do
	if [ -e "${_DIRECTORY}" ]
	then
		for _FILE in "${_DIRECTORY}"/*.sh
		do
			if [ -e "${_FILE}" ]
			then
				. "${_FILE}"
			fi
		done

		break
	fi
done
