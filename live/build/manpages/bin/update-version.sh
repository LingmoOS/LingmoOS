#!/bin/sh

## live-build(7) - Live System Build Components
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


set -e

PROGRAM="LIVE\\\-BUILD"
VERSION="$(cd .. && dpkg-parsechangelog -S Version)"
RELEASE_DATE="$(cd .. && dpkg-parsechangelog -S Timestamp)"

DATE="$(LC_ALL=C date --utc --date="@${RELEASE_DATE}" +%Y\\\\-%m\\\\-%d)"

DAY="$(LC_ALL=C date --utc --date="@${RELEASE_DATE}" +%d)"
MONTH="$(LC_ALL=C date --utc --date="@${RELEASE_DATE}" +%m)"
YEAR="$(LC_ALL=C date --utc --date="@${RELEASE_DATE}" +%Y)"

echo "Updating version headers..."

for MANPAGE in en/*
do
	SECTION="$(basename ${MANPAGE} | awk -F. '{ print $2 }')"

	sed -i -e "s|^.TH.*$|.TH ${PROGRAM} ${SECTION} ${DATE} ${VERSION} \"Debian Live Project\"|" ${MANPAGE}
done

# European date format
for _LANGUAGE in de es fr it
do
	if ls po/${_LANGUAGE}/*.po > /dev/null 2>&1
	then
		for _FILE in po/${_LANGUAGE}/*.po
		do
			sed -i  -e "s|^msgstr .*.2015-.*$|msgstr \"${DAY}.${MONTH}.${YEAR}\"|g" \
				-e "s|^msgstr .*.2015\"$|msgstr \"${DAY}.${MONTH}.${YEAR}\"|g" \
			"${_FILE}"
		done
	fi
done

# Brazilian date format
if ls po/pt_BR/*.po > /dev/null 2>&1
then
	for _FILE in po/pt_BR/*.po
	do
		sed -i  -e "s|^msgstr .*.2015-.*$|msgstr \"${DAY}-${MONTH}-${YEAR}\"|g" \
			-e "s|^msgstr .*-2015\"$|msgstr \"${DAY}-${MONTH}-${YEAR}\"|g" \
		"${_FILE}"
	done
fi
