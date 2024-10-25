#!/bin/sh
# Licensed under the zlib/libpng license (same as NSIS)
#
# Common functions for running tests

MAKENSIS=makensis
OLDPWD="${PWD}"
WINE=wine
WINEDIR=.wine

# Set up wine environment
# Parameters:
#   arch - optional architecture to be used with Wine, default value is win32
#   workdir - optional working directory
#             If not provided then a new temporary directeroy is
#             going to be created.
wine_set_up() {
	WINEARCH=${1:-win32}
	WORKDIR="${2}"
	err=0
	if [ -n "${WORKDIR}" ]; then
		cd "${WORKDIR}"
	else
		cd $(mktemp -d)
	fi
	command -v ${WINE} >/dev/null 2>&1
	err=$?
	if [ ${err} -ne 0 ]; then
		# wine is not available
		err=2
	else
		WINEPREFIX="${PWD}/${WINEDIR}"
		# Disable prompt for mono installer
		WINEDLLOVERRIDES=mscoree=d
		export WINEARCH WINEDLLOVERRIDES WINEPREFIX
		if ${WINE} wineboot; then
			for sig in ALRM HUP INT QUIT TERM USR1; do
				trap "wine_clean_up; after; trap - $sig EXIT; exit 1" "$sig"
			done
		else
			# wineboot failed
			err=$?
			case $(dpkg --print-architecture 2>/dev/null) in
			i386)
				if [ "${WINEARCH}" = "win64" ]; then
					err=95
				fi
				;;
			amd64)
				if [ "${WINEARCH}" = "win32" ]; then
					for arch in $(dpkg \
						--print-foreign-architectures\
						2>/dev/null); do
						if [ ${arch} = "i386" ]; then
							err=95
							break
						fi
					done
					err=95
				fi
				;;
			esac
		fi
	fi
	return ${err}
}

# Clean up wine environment of current working directory in case
# no working directory parameter was passed on
# Parameters:
#   arch - optional architecture used by wine, defaults to win32
#   workdir - optional for retaining working directory
wine_clean_up() {
	WORKDIR="${2}"
	if [ -z "${WORKDIR}" -a -d "${WINEDIR}" ]; then
		# Wait until the currently running wineserver terminates
		wineserver --wait
		rm -rf ${WINEDIR}
		TESTDIR=${PWD}
		if [ "${TESTDIR}" != "${OLDPWD}" ]; then
			# Switch back to old working directory
			cd "${OLDPWD}"
			# If temporary test directory is empty then delete it
			if [ -z "$(ls -A "${TESTDIR}")" ]; then
				rmdir ${TESTDIR}
			fi
		fi
	fi
}

# Convert POSIX to wine path
# Parameter:
#   path - POSIX path
# Returns wine path
wine_convert_path() {
	echo "Z:$1" | sed 's&/&\\&g'
}

# Test that first and second argument are equal
# Parameters:
#   first - first parameter
#   second - second parameter
# Returns 0 if the parameter values are equal
assert_equal() {
	if [ "${1}" != "${2}" ]; then
		echo "${1} != ${2}"
		echo "FAIL"
		return 1
	else
		echo "OK"
	fi
}

# Check and report result
# Parameters:
#   result - return code from check
#   message - message to display
# Exits if result code is not zero
check_result() {
	echo "$2"
	if [ $1 -ne 0 ]; then exit 1; fi
}

# Map wine architecture to the respective NSIS target
# Parameter:
#   arch - architecture used by wine
# Returns makensis compiler option of respective target
map_architecture() {
	case ${WINEARCH} in
		win32)
			echo "-XTarget x86-unicode"
			;;
		win64)
			echo "-XTarget amd64-unicode"
			;;
		*)
			echo ""
			;;
	esac
}

# Generate NSIS installer
# Parameters:
#   additional command line parameters for makensis
make_installer() {
	(cd "${TOPDIR}" && ${MAKENSIS} -NOCD "$(map_architecture)" "$@") \
	|| exit
}

# Run installer
# Parameters:
#   additional command line parameters for installer
run_installer() {
	installer="$1"
	shift
	${WINE} "${installer}" /S $@
}

# Run uninstaller
#   additional command line parameters for uninstaller
run_uninstaller() {
	uninstaller="$1"
	shift
	${WINE} "${uninstaller}" /S $@ \
		_?=$(wine_convert_path \
			$(dirname $(realpath "${uninstaller}")))
}
