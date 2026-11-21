#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


PROGRAM_NAME="live-build"
FRONTEND="lb"
PROGRAM="${FRONTEND} $(basename "${0}")"
VERSION=""
# Find the version:
#  1) For development versions, the git hash with date
#  2) For distributed source code, the version from the changelog
#  3) For installed versions, the version from the file VERSION
if [ ! -z "${LIVE_BUILD}" -a "$(command -v git)" -a -e ${LIVE_BUILD}/.git ]; then
	VERSION="$(cd ${LIVE_BUILD}; git log -n 1 --pretty=format:%H_%aI)"
	# If a local modification is made or there are staged commits, add 'mod'
	# See https://stackoverflow.com/questions/2657935/checking-for-a-dirty-index-or-untracked-files-with-git
	if ! $(cd ${LIVE_BUILD}; git diff-index --quiet HEAD --ignore-submodules --); then
		VERSION="${VERSION}_mod"
	fi
fi
if [ -z "${VERSION}" -a ! -z "${LIVE_BUILD}" -a -e ${LIVE_BUILD}/debian/changelog ]; then
	# Remove the epoch
	VERSION="$(dpkg-parsechangelog -S Version | sed -e 's/^[0-9]://')"
fi
if [ -z "${VERSION}" ]; then
	VERSION="$(cat /usr/share/live/build/VERSION)"
fi

LIVE_BUILD_VERSION="${VERSION}"

PATH="${PWD}/local/bin:${PATH}"
