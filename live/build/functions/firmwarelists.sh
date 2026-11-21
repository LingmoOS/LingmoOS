#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2023 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.

# Updates FIRMWARE_PACKAGES with list of packages determined from specified
# archive areas of specified distro, based upon reading archive content file.
#
# Shared by chroot_firmware and installer_debian-installer
#
# Assumption: firmware packages install files into /lib/firmware
Firmware_List_From_Contents () {
	local MIRROR_CHROOT="${1}"
	local DISTRO_CHROOT="${2}"
	local ARCHIVE_AREAS="${3}"
	local HAS_X86_ARCH=0
	local HAS_ARM_ARCH=0

	local _ARCHIVE_AREA
	for _ARCHIVE_AREA in ${ARCHIVE_AREAS}
	do
		local CONTENTS_FILEDIR="cache/contents.chroot/${DISTRO_CHROOT}/${_ARCHIVE_AREA}"
		mkdir -p "${CONTENTS_FILEDIR}"

		local _ARCH
		for _ARCH in all ${LB_ARCHITECTURE}
		do
			local CONTENTS_URL="${MIRROR_CHROOT}/dists/${DISTRO_CHROOT}/${_ARCHIVE_AREA}/Contents-${_ARCH}.gz"
			local CONTENTS_FILE="${CONTENTS_FILEDIR}/contents-${_ARCH}.gz"

			# Purge from cache if not wanting to use from cache, ensuring fresh copy
			if [ "${LB_CACHE}" != "true" ]
			then
				rm -f "${CONTENTS_FILE}"
			fi

			# If not cached, download
			if [ ! -e "${CONTENTS_FILE}" ]
			then
				# Contents-all.gz does not exist in Buster and other older versions
				if ! wget --quiet --spider ${WGET_OPTIONS} "${CONTENTS_URL}"; then
					continue
				fi
				wget ${WGET_OPTIONS} "${CONTENTS_URL}" -O "${CONTENTS_FILE}"
			fi

			local PACKAGES
			PACKAGES="$(gunzip -c "${CONTENTS_FILE}" | awk '/^(usr\/)?lib\/firmware/ { print $NF }' | sort -u )"
			FIRMWARE_PACKAGES="${FIRMWARE_PACKAGES} ${PACKAGES}"
			if [ -n "${FIRMWARE_DETAILS_FILE}" ]
			then
				# Use similar formatting as tools/make-firmware-image from debian-cd
				# Note: for firmware/Contents-firmware (used by check-missing-firmware.sh from hw-detect),
				#       the second argument must be the filename of the package.
				#       That information is not available here and will be added by installer_debian-installer
				gunzip -c "${CONTENTS_FILE}" | awk -v AREA=${_ARCHIVE_AREA} '/^(usr\/)?lib\/firmware/ { printf "/%-54s %s %s\n", $1, $2, AREA }' >> ${FIRMWARE_DETAILS_FILE}
			fi

			# Don't waste disk space, if not making use of caching
			if [ "${LB_CACHE}" != "true" ]
			then
				rm -f "${CONTENTS_FILE}"
			fi

			case "${_ARCH}" in
				arm64 | armel | armhf)
					HAS_ARM_ARCH=1
					;;
				amd64 | i386)
					HAS_X86_ARCH=1
					;;
			esac
		done

		# Clean up the cache directory, if no files are present
		rmdir --ignore-fail-on-non-empty "cache/contents.chroot/${DISTRO_CHROOT}/${_ARCHIVE_AREA}"
		rmdir --ignore-fail-on-non-empty "cache/contents.chroot/${DISTRO_CHROOT}"
		rmdir --ignore-fail-on-non-empty "cache/contents.chroot"
	done

	# Blocklist firmware which does not match the requested architectures #1035382
	# See https://salsa.debian.org/images-team/debian-cd/-/blob/master/tasks/bookworm/exclude-firmware

	# Filter out firmware packages that are only useful with non-free drivers
	BLOCKLIST_FIRMWARE="firmware-nvidia-gsp firmware-nvidia-tesla-gsp"
	# Exclude ARM firmware when no ARM is requested
	if [ ${HAS_ARM_ARCH} -eq 0 ]
	then
		BLOCKLIST_FIRMWARE="${BLOCKLIST_FIRMWARE} arm-trusted-firmware-tools crust-firmware firmware-qcom-soc firmware-samsung firmware-ti-connectivity raspi-firmware"
	fi
	# Exclude x86 firmware when no x86 is requested
	if [ ${HAS_X86_ARCH} -eq 0 ]
	then
		BLOCKLIST_FIRMWARE="${BLOCKLIST_FIRMWARE} amd64-microcode firmware-intel-sound firmware-sof-signed intel-microcode"
	fi

	# Deduplicate the list and prepare for easier manipulation by having each package on its own line
	local _FIRMWARE_PACKAGES_FILE=tmp_firmware_packages.txt
	echo ${FIRMWARE_PACKAGES} | tr " " "\n" | sort -u > ${_FIRMWARE_PACKAGES_FILE}

	# Remove the blocklisted firmware packages
	# FIRMWARE_PACKAGES has section names and BLOCKLIST_FIRMWARE (intentionally) does not
	local _REMOVEME
	for _REMOVEME in ${BLOCKLIST_FIRMWARE}
	do
		sed -i -e "/\/${_REMOVEME}$/d" ${_FIRMWARE_PACKAGES_FILE}
	done

	# Reassemble the filtered list
	FIRMWARE_PACKAGES=$(cat ${_FIRMWARE_PACKAGES_FILE})
	rm -f ${_FIRMWARE_PACKAGES_FILE}
}
