#!/bin/sh
# Licensed under the zlib/libpng license (same as NSIS)
#
# Script to perform CPUID test
#
# Usage: run.sh [<helper dir [<Windows architecture> [<test dir>]]]

set -e

OUTPUT="output.log"
SCRIPT="cpuid_test.nsi"
SOURCEDIR=$(cd "$(dirname "$0")"; pwd -P)
TOPDIR=$(cd "${SOURCEDIR}/../.."; pwd -P)
HELPERDIR="${1:-"${TOPDIR}/build"}/helpers/${SOURCEDIR##*/}"

INSTALLER="${SCRIPT%.*}.exe"
UNINSTALLER="uninstall.exe"

. "${SOURCEDIR}/../common/funcs.sh"

before() {
	make_installer "-XOutFile ${PWD}/${INSTALLER}" \
		"-DCPUID_HELPER_DIR=${HELPERDIR}" \
		"${SOURCEDIR}/${SCRIPT}"
}

after() {
	[ -f "${OUTPUT}" ] && rm "${OUTPUT}"
	[ -f "${INSTALLER}" ] && rm "${INSTALLER}"
	[ -f "${UNINSTALLER}" ] && rm "${UNINSTALLER}"
}

test_vendor_id() {
        echo "vendor id"
	run_installer ${INSTALLER} "/RESULT=${OUTPUT}" || exit 1
	result=$(assert_equal "${1}" "$(tail -n 1 ${OUTPUT})")
	check_result $? "${result} (Installer)"
	rm -f ${OUTPUT}
	run_uninstaller ${UNINSTALLER} "/RESULT=${OUTPUT}" || exit 1
	result=$(assert_equal "${1}" "$(tail -n 1 ${OUTPUT})")
	check_result $? "${result} (Uninstaller)"
	rm -f ${OUTPUT}
}

main() {
	vendor_id=
	while read -r line
	do
		set -f -- safety $line
		shift
		if [ "${1}" = "vendor_id" ]; then
			vendor_id=${3}
			break
		fi
	done < /proc/cpuinfo
	if [ -n "${vendor_id}" ]; then
		test_vendor_id ${vendor_id}
	fi
}

[ $# -gt 1 ] && shift
wine_set_up "${@}"
if [ "${WINEARCH}" != "win32" ]; then
	echo "CPUID test requiring win32 Windows architecture is skipped"
else
	before
	main
	after
fi
wine_clean_up "${@}"
