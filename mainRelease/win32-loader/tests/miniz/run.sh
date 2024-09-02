#!/bin/sh
# Licensed under the zlib/libpng license (same as NSIS)
#
# Script to perform cpio tests
#
# Usage: run.sh [<helper dir [<Windows architecture> [<test dir>]]]

set -e

CPIO_GZ_NAME="test.cpio.gz"
GZIP_NAME="test.gz"
OUTPUT="output.log"
SCRIPT="miniz_test.nsi"
SOURCEDIR=$(cd "$(dirname "$0")"; pwd -P)
TOPDIR=$(cd "${SOURCEDIR}/../.."; pwd -P)
HELPERDIR="${1:-"${TOPDIR}/build"}/helpers/${SOURCEDIR##*/}"

INSTALLER="${SCRIPT%.*}.exe"
UNINSTALLER="uninstall.exe"

TESTID=

ERROR_SUCCESS=0
ERROR_FILE_NOT_FOUND=2
ERROR_INVALID_DATA=13

. "${SOURCEDIR}/../common/funcs.sh"

before() {
	make_installer "-XOutFile ${PWD}/${INSTALLER}" \
		"-DMINIZ_HELPER_DIR=${HELPERDIR}" \
		"${SOURCEDIR}/${SCRIPT}"
}

after() {
	[ -n "${TESTID}" -a -f "${TESTID}" ] && rm "${TESTID}"
	[ -f "${CPIO_GZ_NAME}" ] && rm "${CPIO_GZ_NAME}"
	[ -f "${GZIP_NAME}" ] && rm "${GZIP_NAME}"
	[ -f "${OUTPUT}" ] && rm "${OUTPUT}"
	[ -f "${INSTALLER}" ] && rm "${INSTALLER}"
	[ -f "${UNINSTALLER}" ] && rm "${UNINSTALLER}"
}

decompress_cpio_gz() {
	gzip --decompress --stdout < "${CPIO_GZ_NAME}" | \
		cpio --extract --to-stdout
}

decompress_gz() {
	gzip --decompress --stdout < "${GZIP_NAME}"
}

check_cpio_gz() {
	result=$1
	[ "${result}" = "${ERROR_SUCCESS}" ] &&	decompress_cpio_gz | \
		diff -q "${t}" - || result=${ERROR_INVALID_DATA}
	result=$(assert_equal "${ERROR_SUCCESS}" "${result}")
	check_result $? "${result} ($2)"
}

check_gzip() {
	result=$1
	[ "${result}" = "${ERROR_SUCCESS}" ] && decompress_gz | \
		diff -q "${t}" - || result=${ERROR_INVALID_DATA}
	result=$(assert_equal "${ERROR_SUCCESS}" "${result}")
	check_result $? "${result} ($2)"
}


test_it() {
	if [ $# -gt 0 ]; then
		t="${1}"
		echo "${t} [gz]"
		run_installer ${INSTALLER} "/IN=${t}" "/OUT=${GZIP_NAME}" \
			"/RESULT=${OUTPUT}" || exit 1
		result=$(tail -n 1 ${OUTPUT})
		check_gzip ${result} "Installer"
		rm -f ${OUTPUT}
		run_uninstaller ${UNINSTALLER} "/IN=${t}" "/OUT=${GZIP_NAME}" \
			"/RESULT=${OUTPUT}" || exit 1
		result=$(tail -n 1 ${OUTPUT})
		check_gzip ${result} "Uninstaller"
		[ -f "${GZIP_NAME}" ] && rm "${GZIP_NAME}"
		rm -f ${OUTPUT}

		echo "${t} [cpio.gz]"
		run_installer ${INSTALLER} "/IN=${t}" \
			"/OUT=${CPIO_GZ_NAME}" "/RESULT=${OUTPUT}" || exit 1
		result=$(tail -n 1 ${OUTPUT})
		check_cpio_gz ${result} "Installer"
		rm -f ${OUTPUT}
		run_uninstaller ${UNINSTALLER} "/IN=${t}" \
			"/OUT=${CPIO_GZ_NAME}" "/RESULT=${OUTPUT}" || exit 1
		result=$(tail -n 1 ${OUTPUT})
		check_cpio_gz ${result} "Uninstaller"
		[ -f "${CPIO_GZ_NAME}" ] && rm "${CPIO_GZ_NAME}"
		[ -f "${t}" ] && rm "${t}"
		rm -f ${OUTPUT}
	fi
}

test_non_existent_file() {
        t="non_existent_file"
	echo "${t} [gz]"
	run_installer ${INSTALLER} "/IN=${t}" "/OUT=${GZIP_NAME}" \
		"/RESULT=${OUTPUT}" || exit 1
	result=$(assert_equal $(tail -n 1 ${OUTPUT}) \
		"${ERROR_FILE_NOT_FOUND}")
	check_result $? "${result} (Installer)"
	rm -f ${OUTPUT}
	run_uninstaller ${UNINSTALLER} "/IN=${t}" "/OUT=${GZIP_NAME}" \
		"/RESULT=${OUTPUT}" || exit 1
	result=$(assert_equal $(tail -n 1 ${OUTPUT}) \
		"${ERROR_FILE_NOT_FOUND}")
	check_result $? "${result} (Uninstaller)"
	rm -f ${OUTPUT}

	echo "${t} [cpio.gz]"
	run_installer ${INSTALLER} "/IN=${t}" "/OUT=${CPIO_GZ_NAME}" \
		"/RESULT=${OUTPUT}" || exit 1
	result=$(assert_equal $(tail -n 1 ${OUTPUT}) \
		"${ERROR_FILE_NOT_FOUND}")
	check_result $? "${result} (Installer)"
	rm -f ${OUTPUT}
	run_uninstaller ${UNINSTALLER} "/IN=${t}" "/OUT=${CPIO_GZ_NAME}" \
		"/RESULT=${OUTPUT}" || exit 1
	result=$(assert_equal $(tail -n 1 ${OUTPUT}) \
		"${ERROR_FILE_NOT_FOUND}")
	check_result $? "${result} (Uninstaller)"
	rm -f ${OUTPUT}
}

test_empty_file() {
        TESTID="empty_file"
	touch ${TESTID}
	test_it "${TESTID}"
}

test_installer() {
	TESTID="installer"
	cp ${INSTALLER} ${TESTID}
	test_it "${TESTID}"
}

test_random() {
	TESTID="random"
	dd if=/dev/urandom iflag=fullblock of=${TESTID} bs=256K count=5
	test_it "${TESTID}"
}

main() {
	test_non_existent_file
	test_empty_file
	test_installer
	test_random
}

[ $# -gt 1 ] && shift
wine_set_up "${@}"
if [ "${WINEARCH}" != "win32" ]; then
	echo "miniz tests requiring win32 Windows architecture are skipped"
else
	before
	main
	after
fi
wine_clean_up "${@}"
