#!/bin/bash

# Rebuild an ISO image for a given timestamp
#
# Copyright 2021-2022 Holger Levsen <holger@layer-acht.org>
# Copyright 2021-2023 Roland Clobus <rclobus@rclobus.nl>
# released under the GPLv2

# Environment variables:
# http_proxy: The proxy that is used by lingmo-system-build and wget
# https_proxy: The proxy that is used by git
# SNAPSHOT_TIMESTAMP: The timestamp to rebuild (format: YYYYMMDD'T'HHMMSS'Z')

# This script can be run as root, but root rights are only required for a few commands.
# You are advised to configure the user with 'visudo' instead.
# Required entries in the sudoers file:
#   Defaults env_keep += "SOURCE_DATE_EPOCH"
#   Defaults env_keep += "LIVE_BUILD"
#   thisuser ALL=(root) NOPASSWD: /usr/bin/lingmo-sys-build build
#   thisuser ALL=(root) NOPASSWD: /usr/bin/lingmo-sys-build clean --purge

# Coding convention: enforced by 'shfmt'

DEBUG=false

set -e
set -o pipefail # see eg http://petereisentraut.blogspot.com/2010/11/pipefail.html

output_echo() {
	set +x
	echo "###########################################################################################"
	echo
	echo -e "$(date -u) - $1"
	echo
	if $DEBUG; then
		set -x
	fi
}

cleanup() {
	output_echo "Generating summary.txt $1"
	cat <<EOF >summary.txt
Configuration: ${CONFIGURATION}
Debian version: ${DEBIAN_VERSION}
Use latest snapshot: ${BUILD_LATEST_DESC}
Installer origin: ${INSTALLER_ORIGIN}
Snapshot timestamp: ${SNAPSHOT_TIMESTAMP}
Snapshot epoch: ${SOURCE_DATE_EPOCH}
Lingmo-system-build override: ${LIVE_BUILD_OVERRIDE}
Lingmo-system-build path: ${LIVE_BUILD}
Build result: ${BUILD_RESULT}
Alternative timestamp: ${PROPOSED_SNAPSHOT_TIMESTAMP}
Checksum: ${SHA256SUM}
Script commandline: ${REBUILD_COMMANDLINE}
Script hash: ${REBUILD_SHA256SUM}
EOF
	touch summary.txt -d@${SOURCE_DATE_EPOCH}
}

show_help() {
	echo "--help, --usage: This help text"
	echo "--configuration: Mandatory, specifies the configuration (desktop environment)"
	echo "--debian-version: Mandatory, e.g. trixie, sid"
	echo "--debian-version-number: The version number, e.g. 13.0.1"
	echo "--debug: Enable debugging output"
	echo "--disk-info: Override the default content for the file .disk/info"
	echo "--generate-source: Enable the building of a source image"
	echo "--installer-origin:"
	echo "    'git' (default): rebuild the installer from git"
	echo "    'archive': take the installer from the Debian archive"
	echo "--timestamp:"
	echo "    'archive' (default): fetches the timestamp from the Debian archive"
	echo "    'snapshot': fetches the latest timestamp from the snapshot server"
	echo "    A timestamp (format: YYYYMMDD'T'HHMMSS'Z'): a specific timestamp on the snapshot server"
}

parse_commandline_arguments() {

	# In alphabetical order
	local LONGOPTS="configuration:,debian-version:,debian-version-number:,debug,disk-info,generate-source,help,installer-origin:,timestamp:,usage"

	local ARGUMENTS
	local ERR=0
	# Add an extra -- to mark the last option
	ARGUMENTS="$(getopt --shell sh --name "${BASH_SOURCE}" --longoptions $LONGOPTS -- -- "${@}")" || ERR=$?

	REBUILD_COMMANDLINE="${@}"
	if [ $ERR -eq 1 ]; then
		output_echo "Error: invalid argument(s)"
		exit 1
	elif [ $ERR -ne 0 ]; then
		output_echo "Error: getopt failure"
		exit 1
	fi
	eval set -- "${ARGUMENTS}"

	while true; do
		local ARG="${1}"
		# In alphabetical order
		case "${ARG}" in
			--configuration)
				shift
				CONFIGURATION=$1
				shift
				;;
			--debian-version)
				shift
				DEBIAN_VERSION=$1
				shift
				;;
			--debian-version-number)
				shift
				DEBIAN_VERSION_NUMBER=$1
				shift
				;;
			--debug)
				shift
				DEBUG=true
				;;
			--disk-info)
				shift
				DISK_INFO="${1}"
				shift
				;;
			--generate-source)
				shift
				GENERATE_SOURCE="--source true"
				;;
			--help|--usage)
				show_help
				exit 0
				;;
			--installer-origin)
				shift
				INSTALLER_ORIGIN=$1
				shift
				;;
			--timestamp)
				shift
				TIMESTAMP=$1
				shift
				;;
			--)
				# The last option
				break
				;;

			*)
				# An earlier version of this script had 2 mandatory arguments and 2 optional arguments
				CONFIGURATION=$1
				shift
				DEBIAN_VERSION=$1
				shift
				if [ "${1}" != "--" ]
				then
					TIMESTAMP=$1
					shift
					if [ "${1}" != "--" ]
					then
						INSTALLER_ORIGIN=$1
						shift
					fi
				fi
				;;
		esac
	done

	case ${CONFIGURATION} in
	"smallest-build")
		INSTALLER="none"
		PACKAGES=""
		;;
	"cinnamon")
		INSTALLER="live"
		PACKAGES="live-task-cinnamon"
		;;
	"gnome")
		INSTALLER="live"
		PACKAGES="live-task-gnome"
		;;
	"kde")
		INSTALLER="live"
		PACKAGES="live-task-kde"
		;;
	"lxde")
		INSTALLER="live"
		PACKAGES="live-task-lxde"
		;;
	"lxqt")
		INSTALLER="live"
		PACKAGES="live-task-lxqt"
		;;
	"mate")
		INSTALLER="live"
		PACKAGES="live-task-mate"
		;;
	"standard")
		INSTALLER="live"
		PACKAGES="live-task-standard"
		;;
	"xfce")
		INSTALLER="live"
		PACKAGES="live-task-xfce"
		;;
	"")
		output_echo "Error: Missing --configuration"
		exit 1
		;;
	*)
		output_echo "Error: Unknown value for --configuration: ${CONFIGURATION}"
		exit 1
		;;
	esac

	# Use 'stable', 'testing' or 'unstable' or code names like 'sid'
	if [ -z "${DEBIAN_VERSION}" ]; then
		output_echo "Error: Missing --debian-version"
		exit 2
	fi
	case "$DEBIAN_VERSION" in
	"bullseye")
		FIRMWARE_ARCHIVE_AREA="non-free contrib"
		;;
	*)
		FIRMWARE_ARCHIVE_AREA="non-free-firmware"
		;;
	esac

	BUILD_LATEST="archive"
	BUILD_LATEST_DESC="yes, from the main Debian archive"
	if [ ! -z "${TIMESTAMP}" ]; then
		case "${TIMESTAMP}" in
		"archive")
			BUILD_LATEST="archive"
			BUILD_LATEST_DESC="yes, from the main Debian archive"
			;;
		"snapshot")
			BUILD_LATEST="snapshot"
			BUILD_LATEST_DESC="yes, from the snapshot server"
			;;
		*)
			SNAPSHOT_TIMESTAMP=${TIMESTAMP}
			BUILD_LATEST="no"
			BUILD_LATEST_DESC="no"
			;;
		esac
	fi

	case "${INSTALLER_ORIGIN}" in
	"git"|"")
		INSTALLER_ORIGIN="git"
		;;
	"archive")
		INSTALLER_ORIGIN="${DEBIAN_VERSION}"
		;;
	*)
		output_echo "Error: Unknown value '${INSTALLER_ORIGIN}' for --installer-origin"
		exit 4
		;;
	esac

	if [ -z "${DEBIAN_VERSION_NUMBER}" ]
	then
		DEBIAN_VERSION_NUMBER=${DEBIAN_VERSION}
	fi

	local CONFIGURATION_SHORT=$(echo ${CONFIGURATION} | cut -c1-2)
	if [ "${CONFIGURATION_SHORT}" == "lx" ]
	then
		# Differentiate between lxqt and lxde
		CONFIGURATION_SHORT=$(echo ${CONFIGURATION} | cut -c1,3)
	fi
	ISO_VOLUME="d-live ${DEBIAN_VERSION_NUMBER} ${CONFIGURATION_SHORT} amd64"

	# Tracing this generator script
	REBUILD_SHA256SUM=$(sha256sum ${BASH_SOURCE} | cut -f1 -d" ")

	if [ $DEBUG ]
	then
		echo "CONFIGURATION = ${CONFIGURATION}"
		echo "DEBIAN_VERSION = ${DEBIAN_VERSION}"
		echo "DEBIAN_VERSION_NUMBER = ${DEBIAN_VERSION_NUMBER}"
		echo "TIMESTAMP = ${TIMESTAMP}"
		echo "SNAPSHOT_TIMESTAMP = ${SNAPSHOT_TIMESTAMP}"
		echo "BUILD_LATEST = ${BUILD_LATEST}"
		echo "BUILD_LATEST_DESC = ${BUILD_LATEST_DESC}"
		echo "INSTALLER_ORIGIN = ${INSTALLER_ORIGIN}"
		echo "ISO_VOLUME = ${ISO_VOLUME}"
		echo "DISK_INFO = ${DISK_INFO}"
	fi
}

get_snapshot_from_archive() {
	wget ${WGET_OPTIONS} http://deb.debian.org/debian/dists/${DEBIAN_VERSION}/InRelease --output-document latest
	#
	# Extract the timestamp from the InRelease file
	#
	# Input:
	# ...
	# Date: Sat, 23 Jul 2022 14:33:45 UTC
	# ...
	# Output:
	# 20220723T143345Z
	#
	SNAPSHOT_TIMESTAMP=$(cat latest | awk '/^Date:/ { print substr($0, 7) }' | xargs -I this_date date --utc --date "this_date" +%Y%m%dT%H%M%SZ)
	rm latest
}

#
# main: follow https://wiki.debian.org/ReproducibleInstalls/LiveImages
#

# Cleanup if something goes wrong
trap cleanup INT TERM EXIT

parse_commandline_arguments "$@"

if $DEBUG; then
	WGET_OPTIONS=
	GIT_OPTIONS=
else
	WGET_OPTIONS=--quiet
	GIT_OPTIONS=--quiet
fi

# No log required
WGET_OPTIONS="${WGET_OPTIONS} --output-file /dev/null --timestamping"

if [ ! -z "${LIVE_BUILD}" ]; then
	LIVE_BUILD_OVERRIDE=1
else
	LIVE_BUILD_OVERRIDE=0
	export LIVE_BUILD=${PWD}/lingmo-system-build
fi

# Prepend sudo for the commands that require it (when not running as root)
if [ "${EUID:-$(id -u)}" -ne 0 ]; then
	SUDO=sudo
fi

# Use a fresh git clone
if [ ! -d ${LIVE_BUILD} -a ${LIVE_BUILD_OVERRIDE} -eq 0 ]; then
	git clone https://salsa.debian.org/live-team/lingmo-system-build.git ${LIVE_BUILD} --single-branch --no-tags
fi

LINGMO_SYS_BUILD_OUTPUT=lingmo-sys-build_output.txt
rm -f ${LINGMO_SYS_BUILD_OUTPUT}

case ${BUILD_LATEST} in
"archive")
	# Use the timestamp of the current Debian archive
	get_snapshot_from_archive
	MIRROR=http://deb.debian.org/debian/
	;;
"snapshot")
	# Use the timestamp of the latest mirror snapshot
	wget ${WGET_OPTIONS} http://snapshot.notset.fr/mr/timestamp/debian/latest --output-document latest
	#
	# Extract the timestamp from the JSON file
	#
	# Input:
	# {
	#   "_api": "0.3",
	#   "_comment": "notset",
	#   "result": "20210828T083909Z"
	# }
	# Output:
	# 20210828T083909Z
	#
	SNAPSHOT_TIMESTAMP=$(cat latest | awk '/"result":/ { split($0, a, "\""); print a[4] }')
	rm latest
	MIRROR=http://snapshot.notset.fr/archive/debian/${SNAPSHOT_TIMESTAMP}
	;;
"no")
	# The value of SNAPSHOT_TIMESTAMP was provided on the command line
	MIRROR=http://snapshot.notset.fr/archive/debian/${SNAPSHOT_TIMESTAMP}
	;;
*)
	echo "E: A new option to BUILD_LATEST has been added"
	exit 1
	;;
esac
# Convert SNAPSHOT_TIMESTAMP to Unix time (insert suitable formatting first)
export SOURCE_DATE_EPOCH=$(date -d $(echo ${SNAPSHOT_TIMESTAMP} | awk '{ printf "%s-%s-%sT%s:%s:%sZ", substr($0,1,4), substr($0,5,2), substr($0,7,2), substr($0,10,2), substr($0,12,2), substr($0,14,2) }') +%s)
output_echo "Info: using the snapshot from ${SOURCE_DATE_EPOCH} (${SNAPSHOT_TIMESTAMP})"

# Use the code from the actual timestamp
# Report the versions that were actually used
if [ ${LIVE_BUILD_OVERRIDE} -eq 0 ]; then
	pushd ${LIVE_BUILD} >/dev/null
	git pull ${GIT_OPTIONS}
	git checkout $(git rev-list -n 1 --min-age=${SOURCE_DATE_EPOCH} HEAD) ${GIT_OPTIONS}
	git clean -Xdf ${GIT_OPTIONS}
	output_echo "Info: using lingmo-system-build from git version $(git log -n 1 --pretty=format:%H_%aI)"
	popd >/dev/null
else
	output_echo "Info: using local lingmo-system-build: $(lingmo-sys-build --version)"
fi

# If the configuration folder already exists, re-create from scratch
if [ -d config ]; then
	${SUDO} lingmo-sys-build clean --purge
	rm -fr config
	rm -fr .build
fi

# Configuration for the live image:
# - For /etc/apt/sources.list: Use the mirror from ${MIRROR}, no security, no updates
# - The debian-installer is built from its git repository, if configured
# - Don't cache the downloaded content
# - Access to non-free-firmware
# - Use an ISO volume label similar to live-wrapper
# - Generate a source image, if configured
# - To reduce some network traffic a proxy is implicitly used
output_echo "Running lingmo-sys-build config."
lingmo-sys-build config \
	--mirror-bootstrap ${MIRROR} \
	--mirror-binary ${MIRROR} \
	--security false \
	--updates false \
	--distribution ${DEBIAN_VERSION} \
	--debian-installer ${INSTALLER} \
	--debian-installer-distribution ${INSTALLER_ORIGIN} \
	--cache-packages false \
	--archive-areas "main ${FIRMWARE_ARCHIVE_AREA}" \
	--iso-volume "${ISO_VOLUME}" \
	${GENERATE_SOURCE} \
	2>&1 | tee $LINGMO_SYS_BUILD_OUTPUT

# Insider knowledge of lingmo-system-build:
#   Add '-o Acquire::Check-Valid-Until=false', to allow for rebuilds of older timestamps
sed -i -e '/^APT_OPTIONS=/s/--yes/--yes -o Acquire::Check-Valid-Until=false/' config/common

if [ ! -z "${PACKAGES}" ]; then
	echo "${PACKAGES}" >config/package-lists/desktop.list.chroot
fi

# Set meta information about the image
mkdir config/includes.binary/.disk
cat << EOF > config/includes.binary/.disk/generator
This image was generated by $(basename ${BASH_SOURCE})
Script commandline: ${REBUILD_COMMANDLINE}
Script hash: ${REBUILD_SHA256SUM}
EOF
ISO8601_TIMESTAMP=$(date --utc -d@${SOURCE_DATE_EPOCH} +%Y-%m-%dT%H:%M:%SZ)
if [ -z "${DISK_INFO}" ]
then
	DISK_INFO="Auto-generated Debian GNU/Linux Live ${DEBIAN_VERSION_NUMBER} ${CONFIGURATION}"
fi
echo -n "${DISK_INFO} ${ISO8601_TIMESTAMP}" > config/includes.binary/.disk/info

# Add additional hooks, that work around known issues regarding reproducibility
cp -a ${LIVE_BUILD}/examples/hooks/reproducible/* config/hooks/normal

# The hook script needs to be escaped once
# The replaced file needs to be escaped twice
cat << EOFHOOK > config/hooks/live/5000-no-password-for-calamares.hook.chroot
#!/bin/sh
set -e

# With live-config < 11.0.4 a password is required for running e.g. Calamares
# See https://bugs.debian.org/cgi-bin/bugreport.cgi?bug=1037295

# Don't run if live-config is not installed
if [ ! -e /usr/lib/live/config/1080-policykit ];
then
  exit 0
fi

# Don't run if the version of 1080-policykit is sufficiently new
if grep -q "/usr/share/polkit-1/rules.d" /usr/lib/live/config/1080-policykit;
then
  exit 0
fi

# Completely replace the content to match the content of 11.0.4
cat << EOFNEWCONTENT > /usr/lib/live/config/1080-policykit
#!/bin/sh

. /lib/live/config.sh

## live-config(7) - System Configuration Components
## Copyright (C) 2016-2023 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


#set -e

Cmdline ()
{
	# Reading kernel command line
	for _PARAMETER in \\\${LIVE_CONFIG_CMDLINE}
	do
		case "\\\${_PARAMETER}" in
			live-config.noroot|noroot)
				LIVE_CONFIG_NOROOT="true"
				;;

			live-config.username=*|username=*)
				LIVE_USERNAME="\\\${_PARAMETER#*username=}"
				;;
		esac
	done
}

Init ()
{
	# Disable root access, no matter what mechanism
	case "\\\${LIVE_CONFIG_NOROOT}" in
		true)
			exit 0
			;;
	esac

	# Checking if package is installed
	if (! pkg_is_installed "polkitd" &&
		! pkg_is_installed "policykit-1") || \\\\
	   component_was_executed "policykit"
	then
		exit 0
	fi

	echo -n " policykit"
}

Config ()
{
	# Configure PolicyKit in live session
	mkdir -p /usr/share/polkit-1/rules.d

	if [ -n "\\\${LIVE_USERNAME}" ]
	then
		cat > /usr/share/polkit-1/rules.d/sudo_on_live.rules << EOF
// Grant the live user access without a prompt
polkit.addRule(function(action, subject) {
	if (subject.local &&
		subject.active &&
		subject.user === "\\\${LIVE_USERNAME}" &&
		subject.isInGroup("sudo")) {
		return polkit.Result.YES;
	}
});
EOF
	else
		cat > /usr/share/polkit-1/rules.d/sudo_on_live.rules << EOF
// Grant the sudo users access without a prompt
polkit.addRule(function(action, subject) {
	if (subject.local &&
		subject.active &&
		subject.isInGroup("sudo")) {
		return polkit.Result.YES;
	}
});
EOF
	fi

	# Creating state file
	touch /var/lib/live/config/policykit
}

Cmdline
Init
Config
EOFNEWCONTENT

echo "P: \$(basename \$0) Bugfix hook has been applied"
EOFHOOK

# For stable and soon-to-be-stable use the same boot splash screen as the Debian installer
case "$DEBIAN_VERSION" in
"bullseye")
	mkdir -p config/bootloaders/syslinux_common
	wget --quiet https://salsa.debian.org/installer-team/debian-installer/-/raw/master/build/boot/artwork/11-homeworld/homeworld.svg -O config/bootloaders/syslinux_common/splash.svg
	mkdir -p config/bootloaders/grub-pc
	ln -s ../../isolinux/splash.png config/bootloaders/grub-pc/splash.png
	;;
"bookworm")
	mkdir -p config/bootloaders/syslinux_common
	wget --quiet https://salsa.debian.org/installer-team/debian-installer/-/raw/master/build/boot/artwork/12-emerald/emerald.svg -O config/bootloaders/syslinux_common/splash.svg
	# To have a 800x600 image and the title 'Live Boot Menu with GRUB', manually undo the title-text modification from binary_syslinux
	cat > config/hooks/live/5010-restore-grub-title.hook.binary << EOF
#!/bin/sh
sed -i -e 's|^title-text:.*|title-text: "Live Boot Menu with GRUB"|' boot/grub/live-theme/theme.txt
EOF
	;;
*)
	# Use the default 'under construction' image
	;;
esac

# Build the image
output_echo "Running lingmo-sys-build build."

set +e # We are interested in the result of 'lingmo-sys-build build', so do not fail on errors
${SUDO} lingmo-sys-build build | tee -a $LINGMO_SYS_BUILD_OUTPUT
BUILD_RESULT=$?
set -e
if [ ${BUILD_RESULT} -ne 0 ]; then
	# Find the snapshot that matches 1 second before the current snapshot
	wget ${WGET_OPTIONS} http://snapshot.notset.fr/mr/timestamp/debian/$(date --utc -d @$((${SOURCE_DATE_EPOCH} - 1)) +%Y%m%dT%H%M%SZ) --output-document but_latest
	PROPOSED_SNAPSHOT_TIMESTAMP=$(cat but_latest | awk '/"result":/ { split($0, a, "\""); print a[4] }')
	rm but_latest

	output_echo "Warning: lingmo-sys-build build failed with ${BUILD_RESULT}. The latest snapshot might not be complete (yet). Try re-running the script with SNAPSHOT_TIMESTAMP=${PROPOSED_SNAPSHOT_TIMESTAMP}."
	# Occasionally the snapshot is not complete, you could use the previous snapshot instead of giving up
	exit 99
fi

# Calculate the checksum
SHA256SUM=$(sha256sum live-image-amd64.hybrid.iso | cut -f 1 -d " ")

if [ ${BUILD_LATEST} == "archive" ]; then
	SNAPSHOT_TIMESTAMP_OLD=${SNAPSHOT_TIMESTAMP}
	get_snapshot_from_archive
	if [ ${SNAPSHOT_TIMESTAMP} != ${SNAPSHOT_TIMESTAMP_OLD} ]; then
		output_echo "Warning: meanwhile the archive was updated. Try re-running the script."
		PROPOSED_SNAPSHOT_TIMESTAMP="${BUILD_LATEST}"
		exit 99
	fi
fi

cleanup success
# Turn off the trap
trap - INT TERM EXIT

# We reached the end, return with PASS
exit 0
