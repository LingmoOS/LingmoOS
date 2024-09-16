#!/bin/sh
# Licensed under the zlib/libpng license (same as NSIS)
#
# Script to perform system information tests
#
# Usage: run.sh [<helper dir [<Windows architecture> [<test dir>]]]

set -e

OUTPUT="output.log"
SCRIPT="sysinfo_test.nsi"
SOURCEDIR=$(cd "$(dirname "$0")"; pwd -P)
TOPDIR=$(cd "${SOURCEDIR}/../.."; pwd -P)
HELPERDIR=${1:-"${TOPDIR}/build"}

INSTALLER="${SCRIPT%.*}.exe"
UNINSTALLER="uninstall.exe"

. "${SOURCEDIR}/../common/funcs.sh"

before() {
	make_installer "-XOutFile ${PWD}/${INSTALLER}" \
		"${SOURCEDIR}/${SCRIPT}"
}

after() {
	[ -f "${OUTPUT}" ] && rm "${OUTPUT}"
	[ -f "${INSTALLER}" ] && rm "${INSTALLER}"
	[ -f "${UNINSTALLER}" ] && rm "${UNINSTALLER}"
}

test_it() {
	if [ $# -gt 1 ]; then
		t="${2}"
		echo "${t}"
		run_installer ${INSTALLER} "/TEST=${t}" \
			"/RESULT=${OUTPUT}" || exit 1
		result=$(assert_equal "${1}" "$(tail -n 1 ${OUTPUT})")
		check_result $? "${result} (Installer)"
		rm -f ${OUTPUT}
		run_uninstaller ${UNINSTALLER} "/TEST=${t}" \
			"/RESULT=${OUTPUT}" || exit 1
		result=$(assert_equal "${1}" "$(tail -n 1 ${OUTPUT})")
		check_result $? "${result} (Uninstaller)"
		rm -f ${OUTPUT}
	fi
}

test_domain() {
    t="Domain"
	domain=$(hostname -d 2>/dev/null)
	if [ -n "${domain}" ]; then
		test_it "${domain}" "${t}"
	else
		echo ${t}
		echo "No domain name"
		echo "SKIPPED"
	fi
}

test_hostname() {
    t="Hostname"
	test_it "$(hostname -s)" "${t}"
}

test_keyboard_layout() {
    t="Keyboard_Layout"
	# United States - English (0x409)
	layout="0x0409"
	test_it "${layout}" "${t}"
}

test_username() {
    t="Username"
	test_it "$(logname 2>/dev/null || echo ${LOGNAME})" "${t}"
}

main() {
	test_domain
	test_hostname
	test_keyboard_layout
	test_username
}

[ $# -gt 1 ] && shift
wine_set_up "${@}"
before
main
after
wine_clean_up "${@}"
