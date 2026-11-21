#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


set -e

# Including common functions
[ -e "${LIVE_BUILD}/scripts/build.sh" ] && . "${LIVE_BUILD}/scripts/build.sh" || . /usr/lib/live/build.sh

# Setting static variables
DESCRIPTION="Install debian-installer into binary"
USAGE="${PROGRAM} [--force]"

# Processing arguments and configuration files
Init_config_data "${@}"

if ! In_list "${LB_DEBIAN_INSTALLER}" cdrom netinst netboot businesscard live; then
	if [ "${LB_DEBIAN_INSTALLER}" = "none" ]; then
		Echo_message "No installer wanted, skipping..."
	fi
	exit 0
fi

Echo_message "Begin installing debian-installer..."

# Requiring stage file
Require_stagefiles config bootstrap

# Checking stage file
Check_stagefile

# Acquire lock file
Acquire_lockfile

# Set d-i image type
case "${LB_DEBIAN_INSTALLER}" in
	businesscard|netboot|netinst)
		DI_IMAGE_TYPE="netboot"
		;;
	*)
		case "${LB_IMAGE_TYPE}" in
			netboot)
				DI_IMAGE_TYPE="netboot"
				;;

			*)
				DI_IMAGE_TYPE="cdrom"
				;;
		esac
		;;
esac

# Checking depends
Check_package host /usr/bin/wget wget
Check_package chroot /usr/bin/apt-ftparchive apt-utils
# Ensure availability of dep11 data
Check_package chroot /etc/apt/apt.conf.d/50appstream appstream

# Restoring cache
Restore_package_cache binary

# Installing depends
Install_packages

# Retrieve dep11 data
Chroot chroot "apt-get update"

if [ "${_DEBUG}" = "true" ]
then
	WGET_OPTIONS="${WGET_OPTIONS} --verbose"
elif [ "${_QUIET}" = "true" ]
then
	WGET_OPTIONS="${WGET_OPTIONS} --quiet"
else
	WGET_OPTIONS="${WGET_OPTIONS} --no-verbose"
fi

# Setting destination directory
case "${LB_IMAGE_TYPE}" in
	netboot)
		DESTDIR="tftpboot/debian-install/${LB_ARCHITECTURE}"
		;;

	*)
		DESTDIR="binary/install"
		;;
esac

# Set architecture-specific variables
case "${LB_ARCHITECTURE}" in
	armel)
		DEFAULT_FLAVOUR="$(echo ${LB_LINUX_FLAVOURS} | awk '{ print $1 }')"
		case "${DI_IMAGE_TYPE}" in
			cdrom)
				DI_REMOTE_BASE="${DEFAULT_FLAVOUR}/cdrom"
				;;

			netboot)
				DI_REMOTE_BASE="${DEFAULT_FLAVOUR}/netboot"
				;;
		esac

		DI_REMOTE_KERNEL="vmlinuz"
		DI_REMOTE_BASE_GTK="${DI_REMOTE_BASE}/gtk"
		;;

	powerpc)
		case "${DI_IMAGE_TYPE}" in
			cdrom)
				DI_REMOTE_BASE="${LB_ARCHITECTURE}/cdrom"
			;;

			netboot)
				DI_REMOTE_BASE="${LB_ARCHITECTURE}/netboot"
				;;
		esac

		DI_REMOTE_KERNEL="vmlinux"
		DI_REMOTE_BASE_GTK="${DI_REMOTE_BASE}/gtk"
		;;

	*)
		case "${DI_IMAGE_TYPE}" in
			netboot)
				DI_REMOTE_BASE="netboot/debian-installer/${LB_ARCHITECTURE}"
				DI_REMOTE_BASE_GTK="netboot/gtk/debian-installer/${LB_ARCHITECTURE}"
				DI_REMOTE_KERNEL="linux"
				;;

			cdrom)
				DI_REMOTE_BASE="cdrom"
				DI_REMOTE_BASE_GTK="cdrom/gtk"
				DI_REMOTE_KERNEL="vmlinuz"
				;;
		esac
		;;
esac

Install_file() {
	local FILE="${1}"

	Echo_debug "Installing file %s" "${FILE}"

	local ARCHIVE_AREA
	ARCHIVE_AREA="$(dpkg -I ${FILE} | awk '/^.*Section: / { print $2 }')"

	if echo "${ARCHIVE_AREA}" | grep -qs '/'
	then
		ARCHIVE_AREA="$(echo ${ARCHIVE_AREA} | awk -F/ '{ print $1 }')"
	else
		ARCHIVE_AREA="main"
	fi

	local TARGET="${2}/${ARCHIVE_AREA}"

	local SOURCE
	SOURCE="$(dpkg -f ${FILE} Source | awk '{ print $1 }')"

	if [ -z "${SOURCE}" ]
	then
		SOURCE="$(basename ${FILE} | awk -F_ '{ print $1 }')"
	fi

	local LETTER
	case "${SOURCE}" in
		lib?*)
			LETTER="$(echo ${SOURCE} | sed 's|\(....\).*|\1|')"
			;;

		*)
			LETTER="$(echo ${SOURCE} | sed 's|\(.\).*|\1|')"
			;;
	esac

	# Install directory
	mkdir -p "${TARGET}"/"${LETTER}"/"${SOURCE}"

	# Move files
	cp -a "${FILE}" "${TARGET}"/"${LETTER}"/"${SOURCE}"
}

# Set absolute directory for caching; we require it when we call Download_file
# from a non-standard cwd.
_LB_CACHE_DIR="$(pwd)/cache/installer_debian-installer"

Prepare_installer_cache() {
	local SRC_FILENAME="${1}"
	local DEST_FILENAME="${_LB_CACHE_DIR}/$(echo "${URL}/${SRC_FILENAME}" | sed 's|/|_|g')"

	mkdir -p $(dirname ${DEST_FILENAME})
	cp -a chroot/debian-installer/build/dest/${SRC_FILENAME} ${DEST_FILENAME}
}

Download_file () {
	local _LB_TARGET="${1}"
	local _LB_URL="${2}"

	Echo_debug "Downloading file \`%s\` from \`%s\`" "${_LB_TARGET}" "${_LB_URL}"

	local _LB_CACHE_FILE
	_LB_CACHE_FILE="${_LB_CACHE_DIR}/$(echo "${_LB_URL}" | sed 's|/|_|g')"

	if [ ! -f "${_LB_CACHE_FILE}" ]
	then
		Echo_debug "Not cached, downloading fresh..."
		mkdir -p ${_LB_CACHE_DIR}
		if ! wget ${WGET_OPTIONS} -O "${_LB_CACHE_FILE}" "${_LB_URL}"
		then
			rm -f "${_LB_CACHE_FILE}"

			Echo_error "Could not download file: %s" "${_LB_URL}"
			exit 1
		fi
	else
		Echo_debug "Using copy from cache..."
	fi

	# Use hardlink if same device
	if [ "$(stat --printf %d "${_LB_CACHE_DIR}/")" = "$(stat --printf %d ./)" ]
	then
		CP_OPTIONS="-l"
	fi

	cp -a -f ${CP_OPTIONS} -- "${_LB_CACHE_FILE}" "${_LB_TARGET}"
}

VMLINUZ_DI="vmlinuz"
INITRD_DI="initrd.gz"
DESTDIR_DI="${DESTDIR}"

VMLINUZ_GI="gtk/vmlinuz"
INITRD_GI="gtk/initrd.gz"
DESTDIR_GI="${DESTDIR}/gtk"

# Debian Installer daily builds
DAILY_DI_URL="https://d-i.debian.org/daily-images/${LB_ARCHITECTURE}/daily"

case "${LB_DERIVATIVE}" in
	true)
		if [ "${LB_DEBIAN_INSTALLER_DISTRIBUTION}" = "daily" ]
		then
			# FIXME: variable name should be decoupled from derivatives
			LB_DEBIAN_INSTALLER_DISTRIBUTION="sid"

			URL="${DAILY_DI_URL}"
		else
			URL="${LB_MIRROR_DEBIAN_INSTALLER}/dists/${LB_DEBIAN_INSTALLER_DISTRIBUTION}/main/installer-${LB_ARCHITECTURE}/current/images"
		fi
		;;

	false)
		if [ "${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}" = "git" -a "${DI_IMAGE_TYPE}" = "cdrom" ]
		then
			LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION="${LB_PARENT_DISTRIBUTION_CHROOT}"
			URL="${LB_PARENT_MIRROR_CHROOT}/snapshot-build_${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}_${LB_ARCHITECTURE}"
			if [ ! -f "${_LB_CACHE_DIR}/$(echo "${URL}/${DI_REMOTE_BASE}/${INITRD_DI}" | sed 's|/|_|g')" ]
			then
				# Packages that are needed for rebuilding the debian installer
				Check_package chroot /lib/module linux-image-generic
				Check_package chroot /usr/bin/git git
				Check_package chroot /usr/bin/mk-build-deps devscripts
				Check_package chroot /usr/bin/equivs-build equivs
				Check_package chroot /usr/bin/gcc gcc
				Install_packages

				# Set architecture-specific variables
				case "${LB_ARCHITECTURE}" in
					arm64)
						TARGETS="build_cdrom_gtk build_cdrom_grub"
						;;

					*)
						TARGETS="build_cdrom_gtk build_cdrom_isolinux"
						;;
				esac

				# These variables do not need to be passed inside the chroot, they can be resolved earlier:
				#   SOURCE_DATE_EPOCH, _QUIET, LB_PARENT_MIRROR_CHROOT, LB_PARENT_DISTRIBUTION_CHROOT
				#   TARGETS, http_proxy
				cat << EOF > chroot/buildit.sh
#!/bin/sh
# Get the version of the git repo that matches SOURCE_DATE_EPOCH

set -e

DEBIAN_INSTALLER=/debian-installer
mkdir -p \${DEBIAN_INSTALLER}
git clone https://salsa.debian.org/installer-team/debian-installer.git \${DEBIAN_INSTALLER} --single-branch --no-tags --quiet
cd \${DEBIAN_INSTALLER}
git checkout \$(git rev-list -n 1 --min-age=${SOURCE_DATE_EPOCH} HEAD) --quiet
if [ "${_QUIET}" != "true" ]
then
	echo "P: using debian-installer from git version \$(git log -n 1 --pretty=format:%H_%aI)"
fi

# Install the build-dependencies, but remove explicit kernel versions,
# because the current kernel version will be used
sed -i '/linux-image-/d' debian/control
mk-build-deps
find -maxdepth 1 -name *.deb | xargs apt-get --yes install

# In kernels since 6.6.8 only a versioned vmlinuz is created
# For older kernels, modify the Makefile to use an existing vmlinuz, if present
# The patch only matches when debian-installer!43 (https://salsa.debian.org/installer-team/debian-installer/-/merge_requests/43) is not merged yet
sed -i -e '/^\\tVMLINUXZ=/s|VMLINUXZ=|if [ ! -e \$(TREE)/boot/vmlinuz ]; then \\\\\\n\\tVMLINUXZ=|' build/Makefile
sed -i -e '/^\\t\\tmv -v \$\$VMLINUXZ/s|; \\\\|; \\\\\\n\\tfi \\\\|' build/Makefile

# Build the installer
cd build
if [ "${_QUIET}" != "true" ]
then
	echo "P: building the debian-installer"
fi
# LINUX_KERNEL_ABI -> use the version of the kernel that matches the current kernel, independent of the version in the git repo (config/common)
#   Output of dpkg-query:
#    ii  linux-image-5.17.0-1-amd64
#    un  linux-image-5.17.0-1-amd64-unsigned
#    ii  linux-image-amd64
#    un  linux-image-generic
#   Or without the extra version number:
#    ii  linux-image-6.6.8-amd64
#    un  linux-image-6.6.8-amd64-unsigned
#    ii  linux-image-amd64
#    un  linux-image-generic
#   -> extract the 5.17.0-1 for LINUX_KERNEL_ABI
#      If there is more than one entry, take the largest (i.e. most recent)
# MIRROR -> our snapshot URL, with disabled expiration
# TARGETS -> only these targets are required
# USE_UDEBS_FROM -> use the same distribution as the chroot
# ROOTCMD -> Workaround for #1058994. Fakeroot is not present in the chroot (and not required)
export LINUX_KERNEL_ABI=\$(dpkg-query --showformat "\\\${db:Status-Abbrev} \\\${Package}\n" --show linux-image-* | awk '\$1=="ii" { c = split(\$2, a, "-"); if (c>3) { if (a[4] ~ /^[0-9]+/) { print a[3] "-" a[4] } else { print a[3] } } }' | sort | tail -1)
http_proxy=${http_proxy} MIRROR="[check-valid-until=no] ${LB_PARENT_MIRROR_CHROOT}" TARGETS="${TARGETS}" USE_UDEBS_FROM=${LB_PARENT_DISTRIBUTION_CHROOT} ROOTCMD=" " bash ./daily-build build-only
EOF
				Chroot chroot "sh buildit.sh"
				if [ ! -e chroot/debian-installer/build/dest/cdrom/"${VMLINUZ_DI}" ];
				then
					Echo_error "The installer could not be built"
					for TARGET in ${TARGETS}; do
						Echo_file chroot/debian-installer/build/dest/${TARGET}.log
					done
					exit 1
				fi
				# Place the files in the cache. Download_file will use the cache instead of downloading
				Prepare_installer_cache cdrom/"${VMLINUZ_DI}"
				Prepare_installer_cache cdrom/"${INITRD_DI}"
				Prepare_installer_cache cdrom/"${VMLINUZ_GI}"
				Prepare_installer_cache cdrom/"${INITRD_GI}"
				rm -rf chroot/debian-installer
				rm -f chroot/buildit.sh
			else
				Echo_debug "Not rebuilding the installer, using copy from cache..."
			fi
		elif [ "${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}" = "daily" ]
		then
			LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION="sid"

			URL="${DAILY_DI_URL}"
		else
			URL="${LB_PARENT_MIRROR_DEBIAN_INSTALLER}/dists/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}/main/installer-${LB_ARCHITECTURE}/current/images"
		fi
esac

mkdir -p "${DESTDIR_DI}"

# Downloading debian-installer
Download_file "${DESTDIR}"/"${VMLINUZ_DI}" ${URL}/${DI_REMOTE_BASE}/${DI_REMOTE_KERNEL}
Download_file "${DESTDIR}"/"${INITRD_DI}" ${URL}/${DI_REMOTE_BASE}/initrd.gz

# Determine the kernel version from the kernel file
# Sample output:
# Linux kernel x86 boot executable bzImage, version 5.19.0-1-amd64 (debian-kernel@lists.debian.org) #1 SMP PREEMPT_DYNAMIC Debian 5.19.6-1 (2022-09-01), RO-rootFS, swap_dev 0X7, Normal VGA
# Linux kernel x86 boot executable bzImage, version 6.12.6-amd64 (debian-kernel@lists.debian.org) #1 SMP PREEMPT_DYNAMIC Debian 6.12.6-1 (2024-12-21), RO-rootFS, swap_dev 0X9, Normal VGA
# Linux kernel x86 boot executable bzImage, version 6.10.9-686 (debian-kernel@lists.debian.org) #1 SMP PREEMPT_DYNAMIC Debian 6.10.9-1 (2024-09-08), RO-rootFS, swap_dev 0X5, Normal VGA
# Linux kernel ARM64 boot executable Image, little-endian, 4K pages
# PE32+ executable (EFI application) RISC-V 64-bit (stripped to external PDB), for MS Windows, 2 sections
if [ "${LB_ARCHITECTURE}" = "amd64" ]
then
	_LINUX_KERNEL_ABI=$(file --brief "${DESTDIR}"/"${VMLINUZ_DI}" | cut -f 8 -d " " | sed -e "s/-${LB_ARCHITECTURE}//")
else
	_LINUX_KERNEL_ABI=""
fi

# Downloading graphical-installer
DOWNLOAD_GTK_INSTALLER=false
if [ "${LB_DEBIAN_INSTALLER_GUI}" = "true" ]
then
	case "${LB_ARCHITECTURE}" in
		amd64|i386|arm64)
			DOWNLOAD_GTK_INSTALLER=true
			;;

		powerpc)
			if [ "${LB_DEBIAN_INSTALLER}" = "netboot" ]
			then
				DOWNLOAD_GTK_INSTALLER=true
			fi
			;;
	esac
fi

if $DOWNLOAD_GTK_INSTALLER; then
	mkdir -p "${DESTDIR_GI}"
	Download_file "${DESTDIR}"/"${VMLINUZ_GI}" ${URL}/${DI_REMOTE_BASE_GTK}/${DI_REMOTE_KERNEL}
	Download_file "${DESTDIR}"/"${INITRD_GI}" ${URL}/${DI_REMOTE_BASE_GTK}/initrd.gz
fi

# Only download additional packages if appropriate
if [ "${DI_IMAGE_TYPE}" != "netboot" ]
then
	# Downloading additional packages
	mkdir -p chroot/binary.deb/archives/partial

	case "${LB_ARCHITECTURE}" in
		amd64)
			DI_REQ_PACKAGES="grub-pc grub-common" # BIOS
			# Starting in trixie, a newer version of libfuse is required
			if [ $(Check_package_available "libfuse3-4") -eq 1 ]
			then
				DI_REQ_PACKAGES="${DI_REQ_PACKAGES} libfuse3-4"
			else
				DI_REQ_PACKAGES="${DI_REQ_PACKAGES} libfuse3-3"
			fi
			# Starting in trixie, the t64-packages have a 64-bit timestamp
			if [ $(Check_package_available "libefiboot1t64") -eq 1 ]
			then
				DI_REQ_PACKAGES="${DI_REQ_PACKAGES} libefiboot1t64"
			else
				DI_REQ_PACKAGES="${DI_REQ_PACKAGES} libefiboot1"
			fi
			if [ $(Check_package_available "libefivar1t64") -eq 1 ]
			then
				DI_REQ_PACKAGES="${DI_REQ_PACKAGES} libefivar1t64"
			else
				DI_REQ_PACKAGES="${DI_REQ_PACKAGES} libefivar1"
			fi
			if [ $(Check_package_available "libfuse2t64") -eq 1 ]
			then
				DI_REQ_PACKAGES="${DI_REQ_PACKAGES} libfuse2t64"
			else
				DI_REQ_PACKAGES="${DI_REQ_PACKAGES} libfuse2"
			fi
			DI_REQ_PACKAGES="${DI_REQ_PACKAGES} grub-efi-amd64 efibootmgr grub-efi-amd64-signed shim-signed" # UEFI (required for d-i, includes suggested packages)
			DI_REQ_PACKAGES="${DI_REQ_PACKAGES} efibootmgr grub-efi grub-efi-amd64 grub-efi-amd64-bin grub-efi-amd64-signed grub2-common mokutil shim-helpers-amd64-signed shim-signed shim-signed-common shim-unsigned cryptsetup cryptsetup-initramfs keyutils" # UEFI (required by Calamares)
			# grub-efi-amd64-unsigned was introduced in trixie
			if [ $(Check_package_available "grub-efi-amd64-unsigned") -eq 1 ]
			then
				DI_REQ_PACKAGES="${DI_REQ_PACKAGES} grub-efi-amd64-unsigned"
			fi
			DI_PACKAGES="linux-image-amd64"
			;;

		i386)
			DI_REQ_PACKAGES="grub-pc"
			DI_PACKAGES="linux-image-686-pae"
			;;

		arm64)
			DI_REQ_PACKAGES="grub-efi-arm64 efibootmgr grub-efi-arm64-signed shim-signed" # UEFI (required for d-i, includes suggested packages)
			DI_REQ_PACKAGES="${DI_REQ_PACKAGES} grub-efi grub-efi-arm64-bin grub2-common mokutil shim-helpers-arm64-signed shim-signed-common shim-unsigned" # UEFI (required by Calamares)
			# grub-efi-arm64-unsigned was introduced in trixie
			if [ $(Check_package_available "grub-efi-arm64-unsigned") -eq 1 ]
			then
				DI_REQ_PACKAGES="${DI_REQ_PACKAGES} grub-efi-arm64-unsigned"
			fi
			DI_PACKAGES="linux-image-arm64"
			;;

		powerpc)
			DI_REQ_PACKAGES="yaboot"
			DI_PACKAGES="linux-image-powerpc linux-image-powerpc64 linux-image-powerpc-smp"
			;;
	esac

	DI_PACKAGES="${DI_PACKAGES} busybox cryptsetup mdadm lvm2 xfsprogs jfsutils"

	case "${LB_MODE}" in
		debian)
			DI_REQ_PACKAGES="${DI_REQ_PACKAGES} console-setup keyboard-configuration kbd"
			;;
	esac

	Echo_debug "DI_REQ_PACKAGES: %s" "${DI_REQ_PACKAGES}"
	Echo_debug "DI_PACKAGES: %s" "${DI_PACKAGES}"

	# The file is generated by Firmware_List_From_Contents, ensure a fresh copy
	FIRMWARE_DETAILS_FILE="installer_firmware_details.txt"
	rm -f ${FIRMWARE_DETAILS_FILE}

	# Include firmware packages
	if [ "${LB_FIRMWARE_BINARY}" = "true" ]
	then
		# Get all firmware packages names
		mkdir -p cache/contents.binary

		FIRMWARE_PACKAGES=""

		# Manually add firmware-linux meta package if available
		if [ $(Check_package_available "firmware-linux") -eq 1 ]
		then
			FIRMWARE_PACKAGES="${FIRMWARE_PACKAGES} firmware-linux"
		fi

		Firmware_List_From_Contents "${LB_PARENT_MIRROR_CHROOT}" "${LB_PARENT_DISTRIBUTION_CHROOT}" "${LB_PARENT_ARCHIVE_AREAS}"

		if [ "${LB_DERIVATIVE}" = "true" ]
		then
			# Avoid downloading things twice if distributions are identical
			DERIVATIVE_ARCHIVE_AREAS=""
			if [ "${LB_DISTRIBUTION_CHROOT}" != "${LB_PARENT_DISTRIBUTION_CHROOT}" ]
			then
				DERIVATIVE_ARCHIVE_AREAS="${LB_ARCHIVE_AREAS}"
			else
				for _ARCHIVE_AREA in ${LB_ARCHIVE_AREAS}
				do
					if ! In_list ${_ARCHIVE_AREA} ${LB_PARENT_ARCHIVE_AREAS}
					then
						DERIVATIVE_ARCHIVE_AREAS="${DERIVATIVE_ARCHIVE_AREAS} ${DERIVATIVE_ARCHIVE_AREAS}"
					fi
				done
			fi

			if [ ! -z "${DERIVATIVE_ARCHIVE_AREAS}" ]
			then
				Firmware_List_From_Contents "${LB_MIRROR_CHROOT}" "${LB_DISTRIBUTION_CHROOT}" "${DERIVATIVE_ARCHIVE_AREAS}"
			fi
		fi

		# Drop section and keep package names only
		for _PACKAGE in ${FIRMWARE_PACKAGES}
		do
			DI_FIRMWARE_PACKAGES="${DI_FIRMWARE_PACKAGES} $(echo ${_PACKAGE} | awk -F/ '{ print $NF }')"
		done
		FIRMWARE_PACKAGES=""
	fi

	Echo_debug "DI_FIRMWARE_PACKAGES: %s" "${DI_FIRMWARE_PACKAGES}"

	# Set apt command prefix
	_LB_APT_COMMAND="apt-get ${APT_OPTIONS} -o Dir::Cache=/binary.deb -o Dir::State::status=/dev/null -o APT::Install-Recommends=false -o Debug::NoLocking=true --download-only"

	# Download .debs of the required packages
	Chroot chroot ${_LB_APT_COMMAND} install ${DI_PACKAGES}
	# The required packages or firmware packages might conflict, so download each individually
	for _PACKAGE in ${DI_FIRMWARE_PACKAGES} ${DI_REQ_PACKAGES}
	do
		Chroot chroot ${_LB_APT_COMMAND} install ${_PACKAGE}
	done

	if [ "${LB_DEBIAN_INSTALLER}" = "live" ]
	then
		# We don't want to duplicate .debs of packages in binary/pool that are already
		# installed to target/ via live-installer.
		#
		# However, we need to force various packages' inclusion in binary/pool as
		# d-i does not support (for example) re-installing grub from target/ - the grub
		# .debs must actually exist.

		# Drop the packages already installed that d-i doesn't explicitly need
		_REMAINING_PACKAGES="$(echo ${DI_FIRMWARE_PACKAGES} ${DI_REQ_PACKAGES} | sed -e 's# #|#g')"
		_REMAINING_PACKAGES="$(Chroot_package_list chroot | grep -E -v "^(${_REMAINING_PACKAGES})\$")"

		Echo_debug "'live' installer mode, dropping following unneeded debs: %s" "${_REMAINING_PACKAGES}"
		for _PACKAGE in ${_REMAINING_PACKAGES}
		do
			rm -f chroot/binary.deb/archives/${_PACKAGE}_*.deb
		done
	fi

	mv chroot/binary.deb ./

	for _ARCHIVE_AREA in ${LB_PARENT_ARCHIVE_AREAS} ${LB_ARCHIVE_AREAS}; do
		mkdir -p binary/pool/${_ARCHIVE_AREA}
	done

	Echo_debug "Installing binary debs"
	if Find_files binary.deb/archives/*.deb
	then
		for FILE in binary.deb/archives/*.deb
		do
			Install_file "${FILE}" "binary/pool"
		done
	fi

	if [ "${LB_DEBIAN_INSTALLER}" != "live" ]
	then
		# Including base debian packages
		if ls cache/packages.bootstrap/*.deb > /dev/null 2>&1
		then
			Echo_debug "Installing bootstrap debs"
			for FILE in cache/packages.bootstrap/*.deb
			do
				Install_file "${FILE}" "binary/pool"
			done
		else
			Echo_error "Could not find packages in cache/packages.bootstrap."
			Echo_error "You selected values of LB_CACHE, LB_CACHE_PACKAGES, LB_CACHE_STAGES and LB_DEBIAN_INSTALLER which will result in 'bootstrap' packages not being cached - these are required when integrating the Debian Installer."
			exit 1
		fi
	fi

	Echo_debug "Including local debs"

	# Including local debs
	if Find_files config/packages.binary/*_"${LB_ARCHITECTURE}".deb || Find_files config/packages/*_"${LB_ARCHITECTURE}".deb
	then
		for FILE in config/packages.binary/*_"${LB_ARCHITECTURE}".deb config/packages/*_"${LB_ARCHITECTURE}".deb
		do
			if [ -e "${FILE}" ]
			then
				Install_file "${FILE}" "binary/pool"
			fi
		done
	fi

	if Find_files config/packages.binary/*_all.deb || Find_files config/packages/*_all.deb
	then
		for FILE in config/packages.binary/*_all.deb config/packages/*_all.deb
		do
			if [ -e "${FILE}" ]
			then
				Install_file "${FILE}" "binary/pool"
			fi
		done
	fi

	# The path to the package pool when running inside the live environment
	case "${LB_INITRAMFS}" in
		live-boot)
			PATH_TO_POOL_INSIDE_LIVE_ENVIRONMENT="/run/live/medium"
			;;

		dracut-live)
			PATH_TO_POOL_INSIDE_LIVE_ENVIRONMENT="/run/initramfs/live"
			;;

		*)
			PATH_TO_POOL_INSIDE_LIVE_ENVIRONMENT=""
			;;
	esac

	# Generating deb indices
	Echo_debug "Generating deb indices"
	for _ARCHIVE_AREA in $(cd binary/pool && ls); do
		INDICE_DIR="dists/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}/${_ARCHIVE_AREA}/binary-${LB_ARCHITECTURE}"
		mkdir -p "binary/${INDICE_DIR}"

		mv binary chroot/root
cat > chroot/binary.sh << EOF
#!/bin/sh
cd /root/binary
apt-ftparchive packages "pool/${_ARCHIVE_AREA}" > "${INDICE_DIR}/Packages"
EOF
		Chroot chroot "sh binary.sh"
		rm -f chroot/binary.sh
		mv chroot/root/binary ./

		touch "binary/${INDICE_DIR}/Packages" -d@${SOURCE_DATE_EPOCH}
		if [ -n "${PATH_TO_POOL_INSIDE_LIVE_ENVIRONMENT}" ]
		then
			# Make the pool available without 'apt-get update'
			VAR_LIB_APT_LISTS_FILENAME=$(echo "${PATH_TO_POOL_INSIDE_LIVE_ENVIRONMENT}/${INDICE_DIR}/Packages" | sed 's|/|_|g')
			cp -a "binary/${INDICE_DIR}/Packages" "chroot/chroot/var/lib/apt/lists/${VAR_LIB_APT_LISTS_FILENAME}"
		fi
		gzip -9 -c "binary/${INDICE_DIR}/Packages" > "binary/${INDICE_DIR}/Packages.gz"

		# Fetching release
		Download_file "binary/${INDICE_DIR}/Release" "${LB_PARENT_MIRROR_CHROOT}/${INDICE_DIR}/Release"
	done

	# Symlink firmware packages to /firmware
	Echo_debug "Symlinking firmware packages"
	if [ -n "${DI_FIRMWARE_PACKAGES}" ]
	then
		mkdir -p binary/firmware
		cd binary/firmware
		# Note: up to (including) bullseye, dep11 was not parsed
		# However, it is easier to have the dep11 code unconditional, making the bullseye (and earlier) images slightly larger than strictly necessary
		_FW_DEP11_DIR=dep11
		mkdir -p ${_FW_DEP11_DIR}
		echo "These files help Debian Installer detect helpful firmware packages (via hw-detect)." > ${_FW_DEP11_DIR}/README.txt

		for _PACKAGE in ${DI_FIRMWARE_PACKAGES}
		do
			for _FILE in $(find ../pool -name "${_PACKAGE}_*.deb")
			do
				ln -sf ${_FILE} ./
				if [ -e ../../${FIRMWARE_DETAILS_FILE} ]
				then
					# Create Contents-firmware similar to tools/make-firmware-image from debian-cd
					# This file is used by check-missing-firmware.sh from hw-detect
					# Note: the second argument is replaced by the filename of the package
					sed --silent -e "s/ [^ ]*\/${_PACKAGE} / "$(basename ${_FILE})" /p" ../../${FIRMWARE_DETAILS_FILE} >> Contents-firmware.unsorted

					_COMPONENT=$(awk '$2 ~ /'${_PACKAGE}'$/ { print $3 }' ../../${FIRMWARE_DETAILS_FILE} | head -1)
					if [ "${_PACKAGE}" = "firmware-sof-signed" ]
					then
						# The script generate_firmware_patterns from debian-cd manually maintains the patterns
						# https://sources.debian.org/src/debian-cd/3.2.1/tools/generate_firmware_patterns/?hl=94#L94
						_LINUX_KERNEL_PACKAGE=$(dpkg-query --showformat "\${db:Status-Abbrev} \${Package}\n" --show linux-image-* | grep "^ii" | grep -v "linux-image-${LB_ARCHITECTURE}" | sort | tail -1 | awk '{ print $2 }')
						if [ -z "${_LINUX_KERNEL_PACKAGE}" ]
						then
							Echo_warning "Could not extract the current kernel version, skipping firmware pattern for ${_PACKAGE}"
						else
							for _ALIAS in $(dpkg-query -L ${_LINUX_KERNEL_PACKAGE} | egrep '(kernel/sound/soc/sof/.*\.ko(.xz)?|/snd-soc-.*sof.*\.ko(.xz)?)$'); do /usr/sbin/modinfo ${_ALIAS} | awk '/^alias:/ { print $2 }'; done | sort -u | awk '{ gsub(/[*]/, ".*", $1); printf "^%s$\n", $1 }' > ${_FW_DEP11_DIR}/${_PACKAGE}.patterns
						fi
					elif [ -n "${_COMPONENT}" ]
					then
						# Find the 'Package:' part, followed by optional 'Provides/modaliases'.
						# If modaliases are found, write 'component' and 'patterns' files
						# This mirrors the functionality from debian-cd (generate_firmware_patterns):
						# - Surround by ^ $
						# - Replace * by .*
						_COMPONENTS_DEP11_FILE=$(echo "${LB_PARENT_MIRROR_DEBIAN_INSTALLER}dists/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}/${_COMPONENT}/dep11/Components-${LB_ARCHITECTURE}.yml.gz" | sed -E 's|https?://||g' | sed 's|/|_|g')
						zcat ../../chroot/var/lib/apt/lists/${_COMPONENTS_DEP11_FILE} | sed -e '/^Package: '${_PACKAGE}'$/,/^---$/p;d' | awk 'BEGIN { modalias_found = 0 } /  modaliases:/ { modalias_found = 1; next } /  [^-]/ { modalias_found = 0; } modalias_found==1 && $2 != "" { gsub(/[*]/, ".*", $2); printf "^%s$\n", $2 }' > ${_FW_DEP11_DIR}/${_PACKAGE}.patterns
					fi
					if [ ! -s ${_FW_DEP11_DIR}/${_PACKAGE}.patterns ]
					then
						# If no modaliases are present, no patterns file is needed
						rm -f ${_FW_DEP11_DIR}/${_PACKAGE}.patterns
					else
						# The third argument contains the component
						echo "${_COMPONENT}" > ${_FW_DEP11_DIR}/${_PACKAGE}.component
					fi
				fi
			done
		done

		LC_ALL=C sort < Contents-firmware.unsorted > Contents-firmware
		rm -f Contents-firmware.unsorted
		cd "${OLDPWD}"
	fi

	# No longer needed
	rm -f ${FIRMWARE_DETAILS_FILE}

	Echo_debug "Generating list of udebs"

	# Prepare a sub directory in the chroot, which can be removed easily at the end
	UDEB_APT_TOPDIR="localArchive"
	APTCONF="aptconf"
	APTDIR="aptdir"

	mkdir -p chroot/${UDEB_APT_TOPDIR}/${APTCONF}/sources.list.d
	mkdir -p chroot/${UDEB_APT_TOPDIR}/${APTCONF}/apt.conf.d
	mkdir -p chroot/${UDEB_APT_TOPDIR}/${APTCONF}/preferences.d
	mkdir -p chroot/${UDEB_APT_TOPDIR}/${APTDIR}/state/lists/partial
	mkdir -p chroot/${UDEB_APT_TOPDIR}/${APTDIR}/cache/archives/partial

	cat > chroot/${UDEB_APT_TOPDIR}/${APTCONF}/sources.list.d/udeb.sources <<-EOF
		Types: deb
		URIs: ${LB_PARENT_MIRROR_CHROOT}
		Suites: ${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}
		Components: main/debian-installer ${LB_PARENT_ARCHIVE_AREAS}
		Signed-by: /usr/share/keyrings/debian-archive-keyring.gpg
		EOF

	# TODO: Untested path, LB_DERIVATIVE is always false
	if [ "${LB_DERIVATIVE}" = "true" ]
	then
		cat > chroot/${UDEB_APT_TOPDIR}/${APTCONF}/sources.list.d/udeb.sources <<-EOF
			Types: deb
			URIs: ${LB_MIRROR_CHROOT}
			Suites: ${LB_DEBIAN_INSTALLER_DISTRIBUTION}
			Components: main/debian-installer ${LB_ARCHIVE_AREAS}
			Signed-by: /usr/share/keyrings/debian-archive-keyring.gpg
			EOF
	fi

	cat > chroot/${UDEB_APT_TOPDIR}/apt.conf <<-EOF
		Dir "${UDEB_APT_TOPDIR}";
		Dir::Etc "${APTCONF}";
		Dir::Cache "${APTDIR}/cache";
		Dir::State "${APTDIR}/state";
		Debug::NoLocking "true";
		Debug::pkgDepCache::AutoInstall "true";
		Acquire::Retries "3";
		APT::Install-Recommends "false";
		Apt::Architecture "${LB_ARCHITECTURE}";
		Apt::Architectures "${LB_ARCHITECTURE}";
		Acquire::Languages "none";
		EOF

	cat > chroot/${UDEB_APT_TOPDIR}/get_udeb_list.sh <<-EOF
		#!/bin/sh
		export APT_CONFIG="/${UDEB_APT_TOPDIR}/apt.conf"
		apt-get update
		EOF

	# Find all available udeb packages
	Chroot chroot "sh ${UDEB_APT_TOPDIR}/get_udeb_list.sh"
	grep "^Package: " chroot/${UDEB_APT_TOPDIR}/${APTDIR}/state/lists/*_debian-installer_*_Packages | cut -d" " -f 2 | sort -u > chroot/${UDEB_APT_TOPDIR}/available_udebs

	Echo_debug "Generating exclude list"

	# Excluding udebs
	# If the same file exists in multiple locations, take the version from the first
	LOCATIONS="/usr/share/live/build/data/debian-cd"
	if [ -n "${LIVE_BUILD}" ]
	then
		LOCATIONS="${LIVE_BUILD}/data/debian-cd ${LOCATIONS}"
	fi
	for LOCATION in ${LOCATIONS}
	do
		if [ -e "${LOCATION}/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}" ] && [ ! -e exclude ]
		then
			grep -v "^#" "${LOCATION}/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}/udeb_exclude" > exclude || true
			grep -v "^#" "${LOCATION}/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}/exclude-udebs" >> exclude || true
			grep -v "^#" "${LOCATION}/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}/exclude-udebs-${LB_ARCHITECTURE}" >> exclude || true
		fi
	done

	# Local exclude file
	if [ -e ../config/debian-installer/udeb_exclude ]
	then
		cat ../config/debian-installer/udeb_exclude >> exclude
	fi

	if [ -e exclude ]
	then
		# Excluding udebs from excludes because we want them to be in the image on purpose
		sed -i -e 's|di-utils-exit-installer||' exclude # used for live-installer-launcher
		if [ "${LB_DEBIAN_INSTALLER}" = "live" ]; then
			sed -i -e 's|live-installer||' exclude
		fi
	else
		Echo_message "No udeb-exclusion list used"
	fi

	cp chroot/${UDEB_APT_TOPDIR}/available_udebs chroot/${UDEB_APT_TOPDIR}/desired_udebs
	if [ -e exclude ]
	then
		# grep -v returns 1 when nothing is selected -> that is OK
		set +e

		# Save bandwidth and space: Don't download what will be thrown away
		if [ -n "${_LINUX_KERNEL_ABI}" ]
		then
			# Filter kernels and modules not matching the current kernel
			grep -E "(modules-.*-di$|^kernel-image-.*-di$)" chroot/${UDEB_APT_TOPDIR}/desired_udebs | grep -v ${_LINUX_KERNEL_ABI} >> exclude
		fi
		LC_ALL=C sort -u < exclude > chroot/${UDEB_APT_TOPDIR}/exclude.sorted
		# First: excludes without wildcard at the end
		grep -v "\*$" chroot/${UDEB_APT_TOPDIR}/exclude.sorted | xargs -IREPLACEME --max-args 1 sed -i -e '/^REPLACEME$/d;' chroot/${UDEB_APT_TOPDIR}/desired_udebs
		# Second: excludes with wildcard at the end
		grep "\*$" chroot/${UDEB_APT_TOPDIR}/exclude.sorted | cut -f 1 -d"*" | xargs -IREPLACEME --max-args 1 sed -i -e '/^REPLACEME/d;' chroot/${UDEB_APT_TOPDIR}/desired_udebs

		# treat non-zero return codes as error again
		set -e
	fi

	# Download the remaining packages (including dependencies)
	cat > chroot/${UDEB_APT_TOPDIR}/download_udebs.sh <<-EOF
		#!/bin/sh
		export APT_CONFIG="/${UDEB_APT_TOPDIR}/apt.conf"
		cat /${UDEB_APT_TOPDIR}/desired_udebs | xargs apt-get -d -y install
		EOF
	Chroot chroot "sh ${UDEB_APT_TOPDIR}/download_udebs.sh"

	_UDEB_POOL="binary.udeb/pool-udeb"
	mkdir -p "${_UDEB_POOL}/main"

	Echo_debug "Including local udebs"

	# Including local udebs
	if Find_files config/packages.binary/*_"${LB_ARCHITECTURE}".udeb || Find_files config/packages/*_"${LB_ARCHITECTURE}".udeb
	then
		for FILE in config/packages.binary/*_"${LB_ARCHITECTURE}".udeb config/packages/*_"${LB_ARCHITECTURE}".udeb
		do
			if [ -e "${FILE}" ]
			then
				Install_file "${FILE}" "${_UDEB_POOL}"

				# Prefer local udebs over downloaded udebs
				sed -i -e "/^$(dpkg -f ${FILE} Package)$/d" chroot/${UDEB_APT_TOPDIR}/desired_udebs
			fi
		done
	fi

	if Find_files config/packages.binary/*_all.udeb || Find_files config/packages/*_all.udeb
	then
		for FILE in config/packages.binary/*_all.udeb config/packages/*_all.udeb
		do
			if [ -e "${FILE}" ]
			then
				Install_file "${FILE}" "${_UDEB_POOL}"

				# Prefer local udebs over downloaded udebs
				sed -i -e "/^$(dpkg -f ${FILE} Package)$/d" chroot/${UDEB_APT_TOPDIR}/desired_udebs
			fi
		done
	fi

	# Sort udebs into alphabetised pool structure
	Echo_debug "Organising into pool structure"
	for UDEB in $(cat chroot/${UDEB_APT_TOPDIR}/desired_udebs)
	do
		FILE="$(realpath chroot/${UDEB_APT_TOPDIR}/${APTDIR}/cache/archives/${UDEB}_*.udeb)"
		if [ -e "${FILE}" ]; then
			Install_file "${FILE}" "${_UDEB_POOL}"
		fi
	done

	# Cleanup downloaded udebs
	rm -fr chroot/${UDEB_APT_TOPDIR}

	# Creating udeb indices
	Echo_debug "Generating udeb indices"

	UDEB_INDICE_DIR="dists/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}/main/debian-installer/binary-${LB_ARCHITECTURE}"

	mkdir -p "binary.udeb/${UDEB_INDICE_DIR}"

	mv binary.udeb chroot/root
	cat > chroot/binary.sh <<-EOF
		#!/bin/sh
		cd /root/binary.udeb
		apt-ftparchive packages "pool-udeb/main" > "${UDEB_INDICE_DIR}/Packages"
		EOF
	Chroot chroot "sh binary.sh"
	rm -f chroot/binary.sh
	mv chroot/root/binary.udeb ./

	cd binary.udeb
	touch "${UDEB_INDICE_DIR}/Packages" -d@${SOURCE_DATE_EPOCH}
	gzip -9 -c "${UDEB_INDICE_DIR}/Packages" > "${UDEB_INDICE_DIR}/Packages.gz"

	find . | cpio -dmpu "${OLDPWD}"/binary
	cd "${OLDPWD}"

	rm -f exclude
	rm -rf binary.udeb
	rm -rf binary.deb

	# Generating release file
	Echo_debug "Generating release file"
	mv binary chroot/root

	if [ -e chroot/etc/os-release ]
	then
		_VERSION="$(. chroot/etc/os-release && echo ${VERSION_ID})"
	fi

	if [ -n "${_VERSION}" ]; then
		_LB_APT_VERSION_OPT='-o APT::FTPArchive::Release::Version="'"${_VERSION}"'"'
	fi

	case "${LB_PARENT_DISTRIBUTION_BINARY}" in
		sid)
			_SUITE="unstable"
			;;

		*)
			_SUITE="${LB_PARENT_DISTRIBUTION_BINARY}"
			;;
	esac

cat > chroot/binary.sh << EOF
#!/bin/sh
cd /root/binary && apt-ftparchive \
	-o APT::FTPArchive::Release::Origin="Debian" \
	-o APT::FTPArchive::Release::Label="Debian" \
	-o APT::FTPArchive::Release::Suite="${_SUITE}" \
	${_LB_APT_VERSION_OPT} \
	-o APT::FTPArchive::Release::Codename="${LB_PARENT_DISTRIBUTION_BINARY}" \
	-o APT::FTPArchive::Release::Date="$(date -R --utc -d@${SOURCE_DATE_EPOCH})" \
	-o APT::FTPArchive::Release::Architectures="${LB_ARCHITECTURE}" \
	-o APT::FTPArchive::Release::Components="${LB_PARENT_ARCHIVE_AREAS}" \
	-o APT::FTPArchive::Release::Description="Last updated: $(date -R --utc -d@${SOURCE_DATE_EPOCH})" \
	release dists/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION} > dists/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}/Release
EOF

	Chroot chroot "sh binary.sh"
	rm -f chroot/binary.sh
	mv chroot/root/binary ./

	if [ -n "${PATH_TO_POOL_INSIDE_LIVE_ENVIRONMENT}" ]
	then
		# Add to sources.list with the path that is used inside the live environment
		# Add in the first line, to give it precedence over the online versions of the same package
		echo "deb [trusted=yes] file:${PATH_TO_POOL_INSIDE_LIVE_ENVIRONMENT} ${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION} ${LB_PARENT_ARCHIVE_AREAS}" > sources.list
		cat chroot/chroot/etc/apt/sources.list >> sources.list
		mv sources.list chroot/chroot/etc/apt/sources.list
		VAR_LIB_APT_LISTS_FILENAME=$(echo "${PATH_TO_POOL_INSIDE_LIVE_ENVIRONMENT}/dists/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}/Release" | sed 's|/|_|g')
		cp -a binary/dists/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}/Release chroot/chroot/var/lib/apt/lists/${VAR_LIB_APT_LISTS_FILENAME}
	fi

	Echo_debug "Creating distribution directories/symlinks"

	DISTRIBUTIONS="stable testing unstable"

	if [ "${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION}" != "${LB_PARENT_DISTRIBUTION_BINARY}" ]
	then
		DISTRIBUTIONS="${DISTRIBUTIONS} ${LB_PARENT_DISTRIBUTION_BINARY}"
	fi

	case "${LB_IMAGE_TYPE}" in
		hdd)
			case "${LB_BINARY_FILESYSTEM}" in
				fat*|ntfs)
					# Creating dist directories
					for DISTRIBUTION in ${DISTRIBUTIONS}
					do
						cp -a binary/dists/${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION} binary/dists/${DISTRIBUTION}
					done
					;;
			esac
			;;

		*)
			# Creating dist symlinks
			for DISTRIBUTION in ${DISTRIBUTIONS}
			do
				ln -s ${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION} binary/dists/${DISTRIBUTION}
			done

			ln -s . binary/debian
			;;
	esac
fi

Repack_initrd()
{
	local TARGET_INITRD="${1}"
	local INCLUDE_PATH="${2}"
	local REPACK_TMPDIR="unpacked-initrd"

	Echo_debug "Repacking initrd \`%s\`" "${TARGET_INITRD}"

	if [ -d "${INCLUDE_PATH}" ]
	then
		INCLUDE_PATH=$(readlink -f ${INCLUDE_PATH})
	fi

	# cpio does not have a "extract to directory", so we must change directory
	mkdir -p ${REPACK_TMPDIR}
	cd ${REPACK_TMPDIR}

	gzip -d < ../${TARGET_INITRD} | cpio -i --make-directories --no-absolute-filenames
	if [ ! -d "${INCLUDE_PATH}" ]
	then
		# Invoked the old way, just copy the preseeds
		local _FILE
		for _FILE in ../config/preseed/*.cfg ../config/preseed/*.cfg.installer
		do
			if [ -e "${_FILE}" ]
			then
				cp "${_FILE}" .
			fi
		done
	else
		# New way, include target directory content in the initrd
		local REPACK_TMPDIR_ABS="${PWD}"
		cd "${INCLUDE_PATH}"
		find -print0 | cpio -pumd0 --no-preserve-owner "${REPACK_TMPDIR_ABS}/"
		cd "${OLDPWD}"
	fi
	find -print0 | cpio -H newc -o0 | gzip -9 > ../${TARGET_INITRD}

	cd ..
	rm -rf ${REPACK_TMPDIR}
}

Echo_debug "Repacking initrds where necessary"

# Preseed d-i by repacking the initrd in certain situations
if [ "${DI_IMAGE_TYPE}" = "netboot" ] && ( ls config/preseed/*.cfg > /dev/null 2>&1 || ls config/preseed/*.cfg.installer > /dev/null 2>&1 )
then
	Repack_initrd "${DESTDIR}"/"${INITRD_DI}"

	if [ -e "${DESTDIR}"/"${INITRD_GI}" ]
	then
		Repack_initrd "${DESTDIR}"/"${INITRD_GI}"
	fi
fi

# Include content of config/includes.installer if exists and not empty
if [ -d config/includes.installer ] && [ -n "$(ls -A config/includes.installer)" ]
then
	Repack_initrd "${DESTDIR}"/"${INITRD_DI}" config/includes.installer

	if [ -e "${DESTDIR}"/"${INITRD_GI}" ]
	then
		Repack_initrd "${DESTDIR}"/"${INITRD_GI}" config/includes.installer
	fi
fi

# Saving cache
Save_package_cache binary

# Removing depends
Remove_packages

# Creating stage file
Create_stagefile
