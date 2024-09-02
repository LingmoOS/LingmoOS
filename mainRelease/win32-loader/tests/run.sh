#!/bin/sh
# Licensed under the zlib/libpng license (same as NSIS)
#
# Script to run all tests
#
# Usage: run.sh [<helper dir [<Windows architecture> [<test dir>]]]

set -e

SOURCEDIR=$(cd "$(dirname "$0")"; pwd -P)
TOPDIR=$(cd "${SOURCEDIR}/.."; pwd -P)
HELPERDIR=${1:-"${TOPDIR}/build"}

. "${SOURCEDIR}/common/funcs.sh"

main() {
       for r in ${SOURCEDIR}/*/run.sh
       do
               $r "${HELPERDIR}" "${WINEARCH}" "${PWD}"
       done
}

after() {
	return 0
}

[ $# -gt 1 ] && shift
err=0
if wine_set_up $@; then
	main
else
	err=$?
	case ${err} in
	2)
		echo "Wine is not available"
		err=0
		;;
	95)
		echo "Wine with ${WINEARCH} architecture is not supported on this host"
		err=0
		;;
	*)
		echo "Failed to boot wine"
		;;
	esac

	if [ ${err} -eq 0 ]; then
		echo "Skip tests"
	fi
fi
wine_clean_up
return ${err}
