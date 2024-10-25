#!/bin/sh
# Licensed under the zlib/libpng license (same as NSIS)
#
# Script to perform SHA256 tests
#
# Usage: run.sh [<helper dir [<Windows architecture> [<test dir>]]]

set -e

OUTPUT="output.log"
SCRIPT="sha256_test.nsi"
SOURCEDIR=$(cd "$(dirname "$0")"; pwd -P)
TOPDIR=$(cd "${SOURCEDIR}/../.."; pwd -P)
HELPERDIR="${1:-"${TOPDIR}/build"}/helpers/${SOURCEDIR##*/}"

INSTALLER="${SCRIPT%.*}.exe"
UNINSTALLER="uninstall.exe"

TESTID=

. "${SOURCEDIR}/../common/funcs.sh"

before() {
	make_installer "-XOutFile ${PWD}/${INSTALLER}" \
		$(test "${WINEARCH}" != "win32" && echo "-DNO_SHA256_HELPER") \
		"-DSHA256_HELPER_DIR=${HELPERDIR}" \
		"${SOURCEDIR}/${SCRIPT}"
}

after() {
	[ -n "${TESTID}" -a -f "${TESTID}" ] && rm "${TESTID}"
	[ -f "${OUTPUT}" ] && rm "${OUTPUT}"
	[ -f "${INSTALLER}" ] && rm "${INSTALLER}"
	[ -f "${UNINSTALLER}" ] && rm "${UNINSTALLER}"
}

run_reference() {
	set -- safety $(sha256sum < "${1}") && shift && echo "${1}"
}

# Convert hexadecimal number into a byte
hex2byte() {
	# Transform hexadecimal number into its octal representation
	n=$((0x${1}))
	divisor=64
	byte=""
	while [ ${divisor} -gt 0 ]
	do
		m=$((${n}/${divisor}))
		n=$((${n}-${m}*${divisor}))
		divisor=$((${divisor}/8))
		byte="${byte}${m}"
	done
	if [ -n "${byte}" ]; then
		byte="\\${byte}"
	fi
	# Create corresponding byte from the octal code
	printf "${byte}"
}

test_sha256() {
	# Skip comment lines starting with #
	if [ -n "${1}" -a "${1}" = "${1#\#}" ]; then
		testcase="Test:"
		# Generate test data according to the provided pattern
		:> "${1}"
		if [ -n "${2}" ]; then
			testcase="${testcase} $2"
			while true
			do
				code=$(dd ibs=1 count=2 2>/dev/null)
				[ -n "${code}" ] || break
				hex2byte ${code} >> "${1}"
			done << EOF
$2
EOF
			if [ -n "${4}" ]; then
				testcase="${testcase} ${3} ${4}"
				transform=""
				if [ "${2}" != "00" ]; then
					transform="| tr \"\0\" \"$(cat ${1})\""
				fi
				eval head -c${4} /dev/zero $transform > "${1}"
			fi
		else
			testcase="${testcase} <empty>"
		fi
		echo "${testcase}"
		# Compute SHA256 digest via Installer and Uninstaller
                # for the generated test data and compare the result
                # with the expected one.
		run_installer ${INSTALLER} \
			"/FILE=${1}" "/RESULT=${OUTPUT}" || exit 1
		result=$(assert_equal "${1}" "$(tail -n 1 ${OUTPUT})")
		check_result $? "${result} (Installer)"
		rm -f ${OUTPUT}
		run_uninstaller ${UNINSTALLER} \
			"/FILE=${1}" "/RESULT=${OUTPUT}" || exit 1
		result=$(assert_equal "${1}" "$(tail -n 1 ${OUTPUT})")
		check_result $? "${result} (Uninstaller)"
		rm -f ${OUTPUT} "${1}"
	fi
}

main() {
	while read -r line
	do
		set -f -- safety $line
		shift
		TESTID="${1}"
		test_sha256 $@
	done < ${SOURCEDIR}/sha256tests.txt
}

[ $# -gt 1 ] && shift
wine_set_up "${@}"
before
main
after
wine_clean_up "${@}"
