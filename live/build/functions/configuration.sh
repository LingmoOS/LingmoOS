#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.


# Prepare config for use, filling in defaults where no value provided for instance
#
# This function should avoid performing validation checks and failing from invalid values.
# Validation should be done through `Validate_config()`.
Prepare_config ()
{
	# Colouring is re-evaluated here just incase a hard coded override was given in the saved config
	case "${_COLOR}" in
		true)
			_COLOR_OUT="true"
			_COLOR_ERR="true"
			;;
		false)
			_COLOR_OUT="false"
			_COLOR_ERR="false"
			;;
		auto)
			;;
	esac
	_BREAKPOINTS="${_BREAKPOINTS:-false}"
	_DEBUG="${_DEBUG:-false}"
	_FORCE="${_FORCE:-false}"
	_QUIET="${_QUIET:-false}"
	_VERBOSE="${_VERBOSE:-false}"

	export LB_CONFIGURATION_VERSION="${LB_CONFIGURATION_VERSION:-${LIVE_BUILD_VERSION}}"
	export LIVE_CONFIGURATION_VERSION="${LB_CONFIGURATION_VERSION}" #for backwards compatibility with hooks

	LB_SYSTEM="${LB_SYSTEM:-live}"

	LB_MODE="${LB_MODE:-debian}"
	LB_DERIVATIVE="false"
	LB_DISTRIBUTION="${LB_DISTRIBUTION:-testing}"
	LB_DISTRIBUTION_CHROOT="${LB_DISTRIBUTION_CHROOT:-${LB_DISTRIBUTION}}"
	LB_DISTRIBUTION_BINARY="${LB_DISTRIBUTION_BINARY:-${LB_DISTRIBUTION_CHROOT}}"

	# Do a reproducible build, i.e. is SOURCE_DATE_EPOCH already set?
	_REPRODUCIBLE="${SOURCE_DATE_EPOCH:-false}"
	if [ "${_REPRODUCIBLE}" != "false" ]; then
		_REPRODUCIBLE=true
	fi
	# For a reproducible build, use UTC per default, otherwise the local time
	_UTC_TIME_DEFAULT=${_REPRODUCIBLE}

	# The current time: for a unified timestamp throughout the building process
 	export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-$(date '+%s')}"

	LB_UTC_TIME="${LB_UTC_TIME:-${_UTC_TIME_DEFAULT}}"
	# Set UTC option for use with `date` where applicable
	if [ "${LB_UTC_TIME}" = "true" ]; then
		DATE_UTC_OPTION="--utc"
	else
		DATE_UTC_OPTION=""
	fi

	export LB_IMAGE_NAME="${LB_IMAGE_NAME:-live-image}"
	export LB_IMAGE_TYPE="${LB_IMAGE_TYPE:-iso-hybrid}"
	#for backwards compatibility with hooks
	export LIVE_IMAGE_NAME="${LB_IMAGE_NAME}"
	export LIVE_IMAGE_TYPE="${LB_IMAGE_TYPE}"

	if [ -z "${LB_ARCHITECTURE}" ]; then
		if command -v dpkg >/dev/null; then
			LB_ARCHITECTURE="$(dpkg --print-architecture)"
		else
			case "$(uname -m)" in
				x86_64)
					LB_ARCHITECTURE="amd64"
					;;

				i?86)
					LB_ARCHITECTURE="i386"
					;;

				*)
					Echo_error "Unable to determine current architecture"
					exit 1
					;;
			esac
		fi
	fi
	export LB_ARCHITECTURE
	# For backwards compat with custom hooks and conditional includes
	export LB_ARCHITECTURES="${LB_ARCHITECTURE}"

	LB_ARCHIVE_AREAS="${LB_ARCHIVE_AREAS:-main}"
	LB_PARENT_ARCHIVE_AREAS="${LB_PARENT_ARCHIVE_AREAS:-${LB_ARCHIVE_AREAS}}"
	export LB_ARCHIVE_AREAS="$(echo "${LB_ARCHIVE_AREAS}" | tr "," " ")"
	export LB_PARENT_ARCHIVE_AREAS="$(echo "${LB_PARENT_ARCHIVE_AREAS}" | tr "," " ")"

	LB_BACKPORTS="${LB_BACKPORTS:-false}"
	if [ -n "$LB_PARENT_DISTRIBUTION" ]; then
		LB_PARENT_DISTRIBUTION_CHROOT="${LB_PARENT_DISTRIBUTION_CHROOT:-${LB_PARENT_DISTRIBUTION}}"
		LB_PARENT_DISTRIBUTION_BINARY="${LB_PARENT_DISTRIBUTION_BINARY:-${LB_PARENT_DISTRIBUTION}}"
		LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION="${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION:-${LB_PARENT_DISTRIBUTION}}"
	else
		LB_PARENT_DISTRIBUTION_CHROOT="${LB_PARENT_DISTRIBUTION_CHROOT:-${LB_DISTRIBUTION_CHROOT}}"
		LB_PARENT_DISTRIBUTION_BINARY="${LB_PARENT_DISTRIBUTION_BINARY:-${LB_DISTRIBUTION_BINARY}}"
		LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION="${LB_PARENT_DEBIAN_INSTALLER_DISTRIBUTION:-${LB_DEBIAN_INSTALLER_DISTRIBUTION}}"
	fi

	LB_APT="${LB_APT:-apt}"
	LB_APT_HTTP_PROXY="${LB_APT_HTTP_PROXY}"
	LB_APT_RECOMMENDS="${LB_APT_RECOMMENDS:-true}"
	LB_APT_SECURE="${LB_APT_SECURE:-true}"
	LB_APT_SOURCE_ARCHIVES="${LB_APT_SOURCE_ARCHIVES:-true}"
	LB_APT_INDICES="${LB_APT_INDICES:-true}"

	APT_OPTIONS="${APT_OPTIONS:---yes -o Acquire::Retries=5}"
	APTITUDE_OPTIONS="${APTITUDE_OPTIONS:---assume-yes -o Acquire::Retries=5}"

	BZIP2_OPTIONS="${BZIP2_OPTIONS:--6}"
	GZIP_OPTIONS="${GZIP_OPTIONS:--6}"
	LZIP_OPTIONS="${LZIP_OPTIONS:--6}"
	LZMA_OPTIONS="${LZMA_OPTIONS:--6}"
	XZ_OPTIONS="${XZ_OPTIONS:--6}"

	if gzip --help | grep -qs "\-\-rsyncable"
	then
		GZIP_OPTIONS="$(echo ${GZIP_OPTIONS} | sed -E -e 's|[ ]?--rsyncable||') --rsyncable"
	fi

	LB_CACHE="${LB_CACHE:-true}"
	if [ "${LB_CACHE}" = "false" ]
	then
		LB_CACHE_INDICES="false"
		LB_CACHE_PACKAGES="false"
		LB_CACHE_STAGES="bootstrap" #bootstrap caching currently required for process to work
	else
		LB_CACHE_INDICES="${LB_CACHE_INDICES:-false}"
		LB_CACHE_PACKAGES="${LB_CACHE_PACKAGES:-true}"
		LB_CACHE_STAGES="${LB_CACHE_STAGES:-bootstrap}"
	fi
	LB_CACHE_STAGES="$(echo "${LB_CACHE_STAGES}" | tr "," " ")"

	LB_DEBCONF_FRONTEND="${LB_DEBCONF_FRONTEND:-noninteractive}"
	LB_DEBCONF_PRIORITY="${LB_DEBCONF_PRIORITY:-critical}"

	case "${LB_SYSTEM}" in
		live)
			LB_INITRAMFS="${LB_INITRAMFS:-live-boot}"
			;;

		normal)
			LB_INITRAMFS="${LB_INITRAMFS:-none}"
			;;
	esac

	LB_INITRAMFS_COMPRESSION="${LB_INITRAMFS_COMPRESSION:-gzip}"

	case "${LB_SYSTEM}" in
		live)
			LB_INITSYSTEM="${LB_INITSYSTEM:-systemd}"
			;;

		normal)
			LB_INITSYSTEM="${LB_INITSYSTEM:-none}"
			;;
	esac

	if [ "${LB_ARCHITECTURE}" = "i386" ] && [ "${CURRENT_IMAGE_ARCHITECTURE}" = "amd64" ]
	then
		# Use linux32 when building amd64 images on i386
		_LINUX32="linux32"
	else
		_LINUX32=""
	fi

	# Mirrors:
	# *_MIRROR_BOOTSTRAP: to fetch packages from
	# *_MIRROR_CHROOT: to fetch packages from
	# *_MIRROR_CHROOT_SECURITY: security mirror to fetch packages from
	# *_MIRROR_BINARY: mirror which ends up in the image
	# *_MIRROR_BINARY_SECURITY: security mirror which ends up in the image
	# *_MIRROR_DEBIAN_INSTALLER: to fetch installer from
	if [ "${LB_MODE}" = "debian" ]; then
		LB_MIRROR_BOOTSTRAP="${LB_MIRROR_BOOTSTRAP:-http://deb.debian.org/debian/}"
		LB_PARENT_MIRROR_BOOTSTRAP="${LB_PARENT_MIRROR_BOOTSTRAP:-${LB_MIRROR_BOOTSTRAP}}"
	fi
	LB_MIRROR_CHROOT="${LB_MIRROR_CHROOT:-${LB_MIRROR_BOOTSTRAP}}"
	LB_PARENT_MIRROR_CHROOT="${LB_PARENT_MIRROR_CHROOT:-${LB_PARENT_MIRROR_BOOTSTRAP}}"
	if [ "${LB_MODE}" = "debian" ]; then
		LB_MIRROR_CHROOT_SECURITY="${LB_MIRROR_CHROOT_SECURITY:-http://security.debian.org/}"
		LB_PARENT_MIRROR_CHROOT_SECURITY="${LB_PARENT_MIRROR_CHROOT_SECURITY:-${LB_MIRROR_CHROOT_SECURITY}}"

		LB_MIRROR_BINARY="${LB_MIRROR_BINARY:-http://deb.debian.org/debian/}"
		LB_PARENT_MIRROR_BINARY="${LB_PARENT_MIRROR_BINARY:-${LB_MIRROR_BINARY}}"

		LB_MIRROR_BINARY_SECURITY="${LB_MIRROR_BINARY_SECURITY:-http://security.debian.org/}"
		LB_PARENT_MIRROR_BINARY_SECURITY="${LB_PARENT_MIRROR_BINARY_SECURITY:-${LB_MIRROR_BINARY_SECURITY}}"
	fi
	LB_MIRROR_DEBIAN_INSTALLER="${LB_MIRROR_DEBIAN_INSTALLER:-${LB_MIRROR_CHROOT}}"
	LB_PARENT_MIRROR_DEBIAN_INSTALLER="${LB_PARENT_MIRROR_DEBIAN_INSTALLER:-${LB_PARENT_MIRROR_CHROOT}}"

	LB_CHROOT_FILESYSTEM="${LB_CHROOT_FILESYSTEM:-squashfs}"

	LB_UNION_FILESYSTEM="${LB_UNION_FILESYSTEM:-overlay}"

	LB_INTERACTIVE="${LB_INTERACTIVE:-false}"

	LB_KEYRING_PACKAGES="${LB_KEYRING_PACKAGES:-debian-archive-keyring}"

	# first, handle existing LB_LINUX_FLAVOURS for backwards compatibility
	if [ -n "${LB_LINUX_FLAVOURS}" ]; then
		LB_LINUX_FLAVOURS_WITH_ARCH="${LB_LINUX_FLAVOURS}"
	fi
	case "${LB_ARCHITECTURE}" in
		arm64)
			LB_LINUX_FLAVOURS_WITH_ARCH="${LB_LINUX_FLAVOURS_WITH_ARCH:-arm64}"
			;;

		armel)
			# armel will have special images: one rootfs image and many additional kernel images.
			# therefore we default to all available armel flavours
			LB_LINUX_FLAVOURS_WITH_ARCH="${LB_LINUX_FLAVOURS_WITH_ARCH:-marvell}"
			;;

		armhf)
			# armhf will have special images: one rootfs image and many additional kernel images.
			# therefore we default to all available armhf flavours
			LB_LINUX_FLAVOURS_WITH_ARCH="${LB_LINUX_FLAVOURS_WITH_ARCH:-armmp armmp-lpae}"
			;;

		amd64)
			LB_LINUX_FLAVOURS_WITH_ARCH="${LB_LINUX_FLAVOURS_WITH_ARCH:-amd64}"
			;;

		i386)
			LB_LINUX_FLAVOURS_WITH_ARCH="${LB_LINUX_FLAVOURS_WITH_ARCH:-686-pae}"
			;;

		ia64)
			LB_LINUX_FLAVOURS_WITH_ARCH="${LB_LINUX_FLAVOURS_WITH_ARCH:-itanium}"
			;;

		powerpc)
			LB_LINUX_FLAVOURS_WITH_ARCH="${LB_LINUX_FLAVOURS_WITH_ARCH:-powerpc64 powerpc}"
			;;

		ppc64el)
			LB_LINUX_FLAVOURS_WITH_ARCH="${LB_LINUX_FLAVOURS_WITH_ARCH:-powerpc64le}"
			;;

		riscv64)
			LB_LINUX_FLAVOURS_WITH_ARCH="${LB_LINUX_FLAVOURS_WITH_ARCH:-riscv64}"
			;;

		s390x)
			LB_LINUX_FLAVOURS_WITH_ARCH="${LB_LINUX_FLAVOURS_WITH_ARCH:-s390x}"
			;;

		*)
			Echo_error "Architecture(s) ${LB_ARCHITECTURE} not yet supported (FIXME)"
			exit 1
			;;
	esac

	LB_LINUX_FLAVOURS=""
	for FLAVOUR in ${LB_LINUX_FLAVOURS_WITH_ARCH}
	do
		ARCH_FILTERED_FLAVOUR="$(echo ${FLAVOUR} | awk -F':' '{print $1}')"
		LB_LINUX_FLAVOURS="${LB_LINUX_FLAVOURS:+$LB_LINUX_FLAVOURS }${ARCH_FILTERED_FLAVOUR}"
	done

	LB_LINUX_PACKAGES="${LB_LINUX_PACKAGES:-linux-image}"

	LB_BINARY_FILESYSTEM="${LB_BINARY_FILESYSTEM:-fat32}"

	case "${LB_PARENT_DISTRIBUTION_BINARY}" in
		sid|unstable)
			LB_SECURITY="${LB_SECURITY:-false}"
			LB_UPDATES="${LB_UPDATES:-false}"
			LB_PROPOSED_UPDATES="${LB_PROPOSED_UPDATES:-false}"
			;;

		*)
			LB_SECURITY="${LB_SECURITY:-true}"
			LB_UPDATES="${LB_UPDATES:-true}"
			LB_PROPOSED_UPDATES="${LB_PROPOSED_UPDATES:-false}"
			;;
	esac

	case "${LB_ARCHITECTURE}" in
		amd64|i386)
			LB_IMAGE_TYPE="${LB_IMAGE_TYPE:-iso-hybrid}"
			;;

		*)
			LB_IMAGE_TYPE="${LB_IMAGE_TYPE:-iso}"
			;;
	esac

	case "${LB_ARCHITECTURE}" in
		amd64|i386)
			if [ "${LB_INITRAMFS}" = "dracut-live" ] && \
				[ "${LB_IMAGE_TYPE}" != "netboot" ]; then
				LB_BOOTLOADER_BIOS="${LB_BOOTLOADER_BIOS:-grub-pc}"
			else
				LB_BOOTLOADER_BIOS="${LB_BOOTLOADER_BIOS:-syslinux}"
			fi
			if ! In_list "${LB_IMAGE_TYPE}" hdd netboot; then
				LB_BOOTLOADER_EFI="${LB_BOOTLOADER_EFI:-grub-efi}"
			fi
			;;
		arm64)
			if ! In_list "${LB_IMAGE_TYPE}" hdd netboot; then
				LB_BOOTLOADER_EFI="${LB_BOOTLOADER_EFI:-grub-efi}"
			fi
			;;
	esac
	# Command line option combines BIOS and EFI selection in one.
	# Also, need to support old config files that held `LB_BOOTLOADERS`.
	# Note that this function does not perform validation, so none is done here.
	if [ -n "${LB_BOOTLOADERS}" ]; then
		LB_BOOTLOADERS="$(echo "${LB_BOOTLOADERS}" | tr "," " ")"
		unset LB_BOOTLOADER_BIOS
		unset LB_BOOTLOADER_EFI
		local BOOTLOADER
		for BOOTLOADER in $LB_BOOTLOADERS; do
			case "${BOOTLOADER}" in
				grub-legacy|grub-pc|syslinux)
					LB_BOOTLOADER_BIOS="${BOOTLOADER}"
					;;
				grub-efi)
					LB_BOOTLOADER_EFI="${BOOTLOADER}"
					;;
			esac
		done
	fi

	if [ "${LB_INITRAMFS}" = "dracut-live" ]; then
		LB_CHECKSUMS="${LB_CHECKSUMS:-md5}"
	else
		LB_CHECKSUMS="${LB_CHECKSUMS:-sha256}"
	fi

	LB_COMPRESSION="${LB_COMPRESSION:-none}"

	LB_ZSYNC="${LB_ZSYNC:-false}"

	LB_BUILD_WITH_CHROOT="${LB_BUILD_WITH_CHROOT:-true}"

	LB_BUILD_WITH_TMPFS="${LB_BUILD_WITH_TMPFS:-false}"

	LB_DEBIAN_INSTALLER="${LB_DEBIAN_INSTALLER:-none}"
	if [ "${LB_DEBIAN_INSTALLER}" = "false" ]
	then
		LB_DEBIAN_INSTALLER="none"
		Echo_warning "A value of 'false' for option LB_DEBIAN_INSTALLER is deprecated, please use 'none' in future."
	fi
	if [ "${LB_DEBIAN_INSTALLER}" = "true" ]
	then
		LB_DEBIAN_INSTALLER="netinst"
		Echo_warning "A value of 'true' for option LB_DEBIAN_INSTALLER is deprecated, please use 'netinst' in future."
	fi

	# cdrom-checker in d-i requires a md5 checksum file
	if [ "${LB_DEBIAN_INSTALLER}" != "none" ]
	then
		if [ "${LB_CHECKSUMS}" = "none" ]
		then
			LB_CHECKSUMS="md5"
		else
			if ! In_list md5 ${LB_CHECKSUMS}; then
				LB_CHECKSUMS=${LB_CHECKSUMS}" md5"
			fi
		fi
	fi

	LB_DEBIAN_INSTALLER_DISTRIBUTION="${LB_DEBIAN_INSTALLER_DISTRIBUTION:-${LB_DISTRIBUTION}}"
	LB_DEBIAN_INSTALLER_GUI="${LB_DEBIAN_INSTALLER_GUI:-true}"

	if [ -z "${LB_DEBIAN_INSTALLER_PRESEEDFILE}" ]
	then
		if Find_files config/debian-installer/preseed.cfg
		then
			LB_DEBIAN_INSTALLER_PRESEEDFILE="/preseed.cfg"
		fi

		if Find_files config/debian-installer/*.cfg && [ ! -e config/debian-installer/preseed.cfg ]
		then
			Echo_warning "You have placed some preseeding files into config/debian-installer but you didn't specify the default preseeding file through LB_DEBIAN_INSTALLER_PRESEEDFILE. This means that debian-installer will not take up a preseeding file by default."
		fi
	fi

	LB_ISO_APPLICATION="${LB_ISO_APPLICATION:-Debian Live}"
	LB_ISO_PREPARER="${LB_ISO_PREPARER:-live-build @LB_VERSION@; https://salsa.debian.org/live-team/live-build}"
	LB_ISO_PUBLISHER="${LB_ISO_PUBLISHER:-Debian Live project; https://wiki.debian.org/DebianLive; debian-live@lists.debian.org}"
	# The string @ISOVOLUME_TS@ must have the same length as the output of `date +%Y%m%d-%H:%M`
	LB_ISO_VOLUME="${LB_ISO_VOLUME:-Debian ${LB_DISTRIBUTION} @ISOVOLUME_TS@}"

	case "${LB_INITRAMFS}" in
		live-boot)
			LB_BOOTAPPEND_LIVE="${LB_BOOTAPPEND_LIVE:-boot=live components quiet splash}"
			LB_BOOTAPPEND_LIVE_FAILSAFE="${LB_BOOTAPPEND_LIVE_FAILSAFE:-boot=live components memtest noapic noapm nodma nomce nosmp nosplash vga=788}"
			;;

		dracut-live)
			# Replace all spaces with underscore for the CD label
			LB_ISO_VOLUME="$(echo "${LB_ISO_VOLUME}" | tr " " "_")"
			LB_BOOTAPPEND_LIVE="${LB_BOOTAPPEND_LIVE:-boot=live components quiet splash rd.live.image root=live:CDLABEL=${LB_ISO_VOLUME} rd.live.dir=live rd.live.squashimg=filesystem.squashfs}"
			LB_BOOTAPPEND_LIVE_FAILSAFE="${LB_BOOTAPPEND_LIVE_FAILSAFE:-boot=live components memtest noapic noapm nodma nomce nosmp nosplash vga=788 rd.live.image root=live:CDLABEL=${LB_ISO_VOLUME} rd.live.dir=live rd.live.squashimg=filesystem.squashfs}"
			;;

		none)
			LB_BOOTAPPEND_LIVE="${LB_BOOTAPPEND_LIVE:-quiet splash}"
			LB_BOOTAPPEND_LIVE_FAILSAFE="${LB_BOOTAPPEND_LIVE_FAILSAFE:-memtest noapic noapm nodma nomce nosmp nosplash vga=788}"
			;;
	esac

	local _LB_BOOTAPPEND_PRESEED
	if [ -n "${LB_DEBIAN_INSTALLER_PRESEEDFILE}" ]
	then
		case "${LB_IMAGE_TYPE}" in
			iso|iso-hybrid)
				_LB_BOOTAPPEND_PRESEED="file=/cdrom/install/${LB_DEBIAN_INSTALLER_PRESEEDFILE}"
				;;

			hdd)
				_LB_BOOTAPPEND_PRESEED="file=/hd-media/install/${LB_DEBIAN_INSTALLER_PRESEEDFILE}"
				;;

			netboot)
				case "${LB_DEBIAN_INSTALLER_PRESEEDFILE}" in
					*://*)
						_LB_BOOTAPPEND_PRESEED="file=${LB_DEBIAN_INSTALLER_PRESEEDFILE}"
						;;

					*)
						_LB_BOOTAPPEND_PRESEED="file=/${LB_DEBIAN_INSTALLER_PRESEEDFILE}"
						;;
				esac
				;;

			tar)
				;;
		esac
	fi

	if [ -n ${_LB_BOOTAPPEND_PRESEED} ]
	then
		LB_BOOTAPPEND_INSTALL="${LB_BOOTAPPEND_INSTALL} ${_LB_BOOTAPPEND_PRESEED}"
	fi

	LB_BOOTAPPEND_INSTALL="$(echo ${LB_BOOTAPPEND_INSTALL} | sed -e 's/[ \t]*$//')"

	LB_HDD_LABEL="${LB_HDD_LABEL:-DEBIAN_LIVE}"
	LB_HDD_SIZE="${LB_HDD_SIZE:-auto}"

	LB_MEMTEST="${LB_MEMTEST:-none}"
	if [ "${LB_MEMTEST}" = "false" ]; then
		LB_MEMTEST="none"
		Echo_warning "A value of 'false' for option LB_MEMTEST is deprecated, please use 'none' in future."
	fi

	case "${LB_ARCHITECTURE}" in
		amd64|i386)
			if [ "${LB_DEBIAN_INSTALLER}" != "none" ]; then
				LB_LOADLIN="${LB_LOADLIN:-true}"
			else
				LB_LOADLIN="${LB_LOADLIN:-false}"
			fi
			;;

		*)
			LB_LOADLIN="${LB_LOADLIN:-false}"
			;;
	esac

	LB_WIN32_LOADER="${LB_WIN32_LOADER:-false}"

	LB_NET_TARBALL="${LB_NET_TARBALL:-true}"

	LB_ONIE="${LB_ONIE:-false}"
	LB_ONIE_KERNEL_CMDLINE="${LB_ONIE_KERNEL_CMDLINE:-}"

	LB_FIRMWARE_CHROOT="${LB_FIRMWARE_CHROOT:-true}"
	LB_FIRMWARE_BINARY="${LB_FIRMWARE_BINARY:-true}"

	LB_SWAP_FILE_SIZE="${LB_SWAP_FILE_SIZE:-512}"

	LB_UEFI_SECURE_BOOT="${LB_UEFI_SECURE_BOOT:-auto}"

	LB_SOURCE="${LB_SOURCE:-false}"
	LB_SOURCE_IMAGES="${LB_SOURCE_IMAGES:-tar}"
	LB_SOURCE_IMAGES="$(echo "${LB_SOURCE_IMAGES}" | tr "," " ")"

	# Foreign/port bootstrapping
	if [ -n "${LB_BOOTSTRAP_QEMU_ARCHITECTURES}" ]; then
		LB_BOOTSTRAP_QEMU_ARCHITECTURE="${LB_BOOTSTRAP_QEMU_ARCHITECTURES}"
		unset LB_BOOTSTRAP_QEMU_ARCHITECTURES
		Echo_warning "LB_BOOTSTRAP_QEMU_ARCHITECTURES was renamed to LB_BOOTSTRAP_QEMU_ARCHITECTURE, please update your config."
	fi
	LB_BOOTSTRAP_QEMU_ARCHITECTURE="${LB_BOOTSTRAP_QEMU_ARCHITECTURE:-}"
	LB_BOOTSTRAP_QEMU_EXCLUDE="${LB_BOOTSTRAP_QEMU_EXCLUDE:-}"
	LB_BOOTSTRAP_QEMU_STATIC="${LB_BOOTSTRAP_QEMU_STATIC:-}"
}

Validate_config ()
{
	Validate_config_permitted_values
	Validate_config_dependencies
}

# Check values are individually permitted, including:
#  - value in list of available values
#  - string lengths within permitted ranges
Validate_config_permitted_values ()
{
	if [ "${LB_APT_INDICES}" != "true" ] && [ "${LB_APT_INDICES}" != "false" ]; then
		Echo_error "Value for LB_APT_INDICES (--apt-indices) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_APT_RECOMMENDS}" != "true" ] && [ "${LB_APT_RECOMMENDS}" != "false" ]; then
		Echo_error "Value for LB_APT_RECOMMENDS (--apt-recommends) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_APT_SECURE}" != "true" ] && [ "${LB_APT_SECURE}" != "false" ]; then
		Echo_error "Value for LB_APT_SECURE (--apt-secure) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_APT_SOURCE_ARCHIVES}" != "true" ] && [ "${LB_APT_SOURCE_ARCHIVES}" != "false" ]; then
		Echo_error "Value for LB_APT_SOURCE_ARCHIVES (--apt-source-archives) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_BACKPORTS}" != "true" ] && [ "${LB_BACKPORTS}" != "false" ]; then
		Echo_error "Value for LB_BACKPORTS (--backports) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_BUILD_WITH_CHROOT}" != "true" ] && [ "${LB_BUILD_WITH_CHROOT}" != "false" ]; then
		Echo_error "Value for LB_BUILD_WITH_CHROOT (--build-with-chroot) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_CACHE}" != "true" ] && [ "${LB_CACHE}" != "false" ]; then
		Echo_error "Value for LB_CACHE (--cache) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_CACHE_INDICES}" != "true" ] && [ "${LB_CACHE_INDICES}" != "false" ]; then
		Echo_error "Value for LB_CACHE_INDICES (--cache-indices) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_CACHE_PACKAGES}" != "true" ] && [ "${LB_CACHE_PACKAGES}" != "false" ]; then
		Echo_error "Value for LB_CACHE_PACKAGES (--cache-packages) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_DEBIAN_INSTALLER_GUI}" != "true" ] && [ "${LB_DEBIAN_INSTALLER_GUI}" != "false" ]; then
		Echo_error "Value for LB_DEBIAN_INSTALLER_GUI (--debian-installer-gui) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_FIRMWARE_BINARY}" != "true" ] && [ "${LB_FIRMWARE_BINARY}" != "false" ]; then
		Echo_error "Value for LB_FIRMWARE_BINARY (--firmware-binary) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_FIRMWARE_CHROOT}" != "true" ] && [ "${LB_FIRMWARE_CHROOT}" != "false" ]; then
		Echo_error "Value for LB_FIRMWARE_CHROOT (--firmware-chroot) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_LOADLIN}" != "true" ] && [ "${LB_LOADLIN}" != "false" ]; then
		Echo_error "Value for LB_LOADLIN (--loadlin) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_NET_TARBALL}" != "true" ] && [ "${LB_NET_TARBALL}" != "false" ]; then
		Echo_error "Value for LB_NET_TARBALL (--net-tarball) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_ONIE}" != "true" ] && [ "${LB_ONIE}" != "false" ]; then
		Echo_error "Value for LB_ONIE (--onie) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_PROPOSED_UPDATES}" != "true" ] && [ "${LB_PROPOSED_UPDATES}" != "false" ]; then
		Echo_error "Value for LB_PROPOSED_UPDATES (--proposed-updates) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_SECURITY}" != "true" ] && [ "${LB_SECURITY}" != "false" ]; then
		Echo_error "Value for LB_SECURITY (--security) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_SOURCE}" != "true" ] && [ "${LB_SOURCE}" != "false" ]; then
		Echo_error "Value for LB_SOURCE (--source) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_UPDATES}" != "true" ] && [ "${LB_UPDATES}" != "false" ]; then
		Echo_error "Value for LB_UPDATES (--updates) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_UTC_TIME}" != "true" ] && [ "${LB_UTC_TIME}" != "false" ]; then
		Echo_error "Value for LB_UTC_TIME (--utc-time) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_WIN32_LOADER}" != "true" ] && [ "${LB_WIN32_LOADER}" != "false" ]; then
		Echo_error "Value for LB_WIN32_LOADER (--win32-loader) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LB_ZSYNC}" != "true" ] && [ "${LB_ZSYNC}" != "false" ]; then
		Echo_error "Value for LB_ZSYNC (--zsync) can only be 'true' or 'false'!"
		exit 1
	fi

	if ! In_list "${LB_APT}" apt apt-get aptitude; then
		Echo_error "You have specified an invalid value for LB_APT (--apt)."
		exit 1
	fi

	if ! In_list "${LB_BINARY_FILESYSTEM}" fat16 fat32 ext2 ext3 ext4 ntfs; then
		Echo_error "You have specified an invalid value for LB_BINARY_FILESYSTEM (--binary-filesystem)."
		exit 1
	fi

	if ! In_list "${LB_IMAGE_TYPE}" iso iso-hybrid hdd tar netboot; then
		Echo_error "You have specified an invalid value for --binary-image."
		exit 1
	fi

	if [ -z "${LB_BOOTLOADER_BIOS}" ] && [ -z "${LB_BOOTLOADER_EFI}" ]; then
		Echo_warning "You have specified no bootloaders; I predict that you will experience some problems!"
	fi
	if [ -n "${LB_BOOTLOADER_BIOS}" ] && ! In_list "${LB_BOOTLOADER_BIOS}" grub-legacy grub-pc syslinux; then
		Echo_error "You have specified an invalid BIOS bootloader."
		exit 1
	fi
	if [ -n "${LB_BOOTLOADER_EFI}" ] && ! In_list "${LB_BOOTLOADER_EFI}" grub-efi; then
		Echo_error "You have specified an invalid EFI bootloader."
		exit 1
	fi
	if [ -n "${LB_BOOTLOADERS}" ]; then
		local BOOTLOADER
		local BOOTLOADERS_BIOS=0
		local BOOTLOADERS_EFI=0
		for BOOTLOADER in $LB_BOOTLOADERS; do
			# Note, multiple instances of the same bootloader should be rejected,
			# to avoid issues (e.g. in `binary_iso` bootloader handling).
			case "${BOOTLOADER}" in
				grub-legacy|grub-pc|syslinux)
					BOOTLOADERS_BIOS=$(( $BOOTLOADERS_BIOS + 1 ))
					;;
				grub-efi)
					BOOTLOADERS_EFI=$(( $BOOTLOADERS_EFI + 1 ))
					;;
				*)
					Echo_error "The following is not a valid bootloader: '%s'" "${BOOTLOADER}"
					exit 1
					;;
			esac
		done
		if [ $BOOTLOADERS_BIOS -ge 2 ]; then
			Echo_error "Invalid bootloader selection. Multiple BIOS instances specified."
			exit 1
		fi
		if [ $BOOTLOADERS_EFI -ge 2 ]; then
			Echo_error "Invalid bootloader selection. Multiple EFI instances specified."
			exit 1
		fi
		if [ $BOOTLOADERS_BIOS -eq 0 ] && [ $BOOTLOADERS_EFI -eq 0 ]; then
			Echo_warning "You have specified no bootloaders; I predict that you will experience some problems!"
		fi
	fi

	local CACHE_STAGE
	for CACHE_STAGE in ${LB_CACHE_STAGES}; do
		if ! In_list "${CACHE_STAGE}" bootstrap chroot rootfs; then
			Echo_warning "The following is not a valid stage: '%s'" "${CACHE_STAGE}"
		fi
	done

	local CHECKSUM
	if [ "${LB_CHECKSUMS}" != "none" ]; then
		for CHECKSUM in ${LB_CHECKSUMS}; do
			if ! In_list "${CHECKSUM}" md5 sha1 sha224 sha256 sha384 sha512; then
				Echo_error "You have specified an invalid value for LB_CHECKSUMS (--checksums): '%s'" "${CHECKSUM}"
				exit 1
			fi
		done
	fi

	if ! In_list "${LB_CHROOT_FILESYSTEM}" ext2 ext3 ext4 squashfs jffs2 none plain; then
		Echo_error "You have specified an invalid value for LB_CHROOT_FILESYSTEM (--chroot-filesystem)."
		exit 1
	fi

	if ! In_list "${LB_COMPRESSION}" bzip2 gzip lzip xz none; then
		Echo_error "You have specified an invalid value for LB_COMPRESSION (--compression)."
		exit 1
	fi

	if ! In_list "${LB_DEBCONF_FRONTEND}" dialog editor noninteractive readline; then
		Echo_error "You have specified an invalid value for LB_DEBCONF_FRONTEND (--debconf-frontend)."
		exit 1
	fi

	if ! In_list "${LB_DEBCONF_PRIORITY}" low medium high critical; then
		Echo_error "You have specified an invalid value for LB_DEBCONF_PRIORITY (--debconf-priority)."
		exit 1
	fi

	if ! In_list "${LB_DEBIAN_INSTALLER}" cdrom netinst netboot businesscard live none; then
		Echo_error "You have specified an invalid value for LB_DEBIAN_INSTALLER (--debian-installer)."
		exit 1
	fi

	if echo ${LB_HDD_LABEL} | grep -qs ' '; then
		Echo_error "Whitespace is not currently supported in HDD labels (LB_HDD_LABEL; --hdd-label)."
		exit 1
	fi

	if ! In_list "${LB_INITRAMFS}" none live-boot dracut-live; then
		Echo_error "You have specified an invalid value for LB_INITRAMFS (--initramfs)."
		exit 1
	fi

	if [ "${LB_INITRAMFS}" = "dracut-live" ]; then
		if [ "${LB_DM_VERITY}"  = "true" ]; then
			Echo_error "Currently unsupported/untested: dm_verity and dracut."
			exit 1
		fi
		if [ "${LB_BOOTLOADER_BIOS}" = "grub-legacy" ]; then
			Echo_error "Currently unsupported/untested: grub-legacy and dracut."
			exit 1
		fi
		if [ "${LB_BOOTLOADER_BIOS}" = "syslinux" ] && \
				[ "${LB_IMAGE_TYPE}" != "netboot" ]; then
			Echo_error "Currently unsupported/untested: syslinux and dracut without netboot."
			exit 1
		fi
		if ! In_list "${LB_IMAGE_TYPE}" iso iso-hybrid netboot; then
			# The boot=live:CDLABEL requires a CD medium
			Echo_error "Currently unsupported/untested: image type ${LB_IMAGE_TYPE} and dracut."
			exit 1
		fi
		if [ "${LB_INITRAMFS_COMPRESSION}" != "gzip" ]; then
			Echo_error "Currently unsupported/untested: compression ${LB_INITRAMFS_COMPRESSION} and dracut."
			exit 1
		fi
		if [ "${LB_CHROOT_FILESYSTEM}" != "squashfs" ]; then
			Echo_error "Currently unsupported/untested: chroot filesystem ${LB_CHROOT_FILESYSTEM} and dracut."
			exit 1
		fi
		if [ "${LB_INITSYSTEM}" != systemd ]; then
			Echo_error "Currently unsupported/untested: init system ${LB_INITSYSTEM} and dracut."
			exit 1
		fi
	fi

	if ! In_list "${LB_INITRAMFS_COMPRESSION}" bzip2 gzip lzma; then
		Echo_error "You have specified an invalid value for LB_INITRAMFS_COMPRESSION (--initramfs-compression)."
		exit 1
	fi

	if ! In_list "${LB_INITSYSTEM}" sysvinit systemd none; then
		Echo_error "You have specified an invalid value for LB_INITSYSTEM (--initsystem)."
		exit 1
	fi

	if ! In_list "${LB_INTERACTIVE}" true shell x11 xnest false; then
		Echo_error "You have specified an invalid value for LB_INTERACTIVE (--interactive)."
		exit 1
	fi

	if [ "$(echo -n "${LB_ISO_APPLICATION}" | wc -c)" -gt 128 ]; then
		Echo_warning "You have specified a value of LB_ISO_APPLICATION (--iso-application) that is too long; the maximum length is 128 characters."
	fi

	if [ "$(echo -n "${LB_ISO_PREPARER}" | sed -e "s/@LB_VERSION@/${VERSION}/" | wc -c)" -gt  128 ]; then
		Echo_warning "You have specified a value of LB_ISO_PREPARER (--iso-preparer) that is too long; the maximum length is 128 characters."
	fi

	if [ "$(echo -n "${LB_ISO_PUBLISHER}" | wc -c)" -gt 128 ]; then
		Echo_warning "You have specified a value of LB_ISO_PUBLISHER (--iso-publisher) that is too long; the maximum length is 128 characters."
	fi

	if [ "$(echo -n "${LB_ISO_VOLUME}" | sed -e "s/@ISOVOLUME_TS@/$(date $DATE_UTC_OPTION -d@${SOURCE_DATE_EPOCH} +%Y%m%d-%H:%M)/" | wc -c)" -gt 32 ]; then
		Echo_warning "You have specified a value of LB_ISO_VOLUME (--iso-volume) that is too long; the maximum length is 32 characters."
	fi

	if ! In_list "${LB_MEMTEST}" memtest86+ memtest86 none; then
		Echo_error "You have specified an invalid value for LB_MEMTEST (--memtest)."
		exit 1
	fi

	if ! In_list "${LB_SOURCE_IMAGES}" iso netboot tar hdd; then
		Echo_error "You have specified an invalid value for LB_SOURCE_IMAGES (--source-images)."
		exit 1
	fi

	if ! In_list "${LB_SYSTEM}" live normal; then
		Echo_error "You have specified an invalid value for LB_SYSTEM (--system)."
		exit 1
	fi

	if ! In_list "${LB_UEFI_SECURE_BOOT}" auto enable disable; then
		Echo_error "You have specified an invalid value for LB_UEFI_SECURE_BOOT (--uefi-secure-boot)."
		exit 1
	fi

	if [ -n "${LB_BOOTSTRAP_QEMU_ARCHITECTURE}" ]; then
		if [ -z "${LB_BOOTSTRAP_QEMU_STATIC}" ]; then
			Echo_error "You have not specified the qemu-static binary for ${LB_BOOTSTRAP_QEMU_ARCHITECTURE} (--bootstrap-qemu-static)"
			exit 1
		fi
		if [ ! -e "${LB_BOOTSTRAP_QEMU_STATIC}" ]; then
			Echo_error "The qemu-static binary (${LB_BOOTSTRAP_QEMU_STATIC}) for ${LB_BOOTSTRAP_QEMU_ARCHITECTURE} was not found on the host"
			exit 1
		fi

		if [ ! -x "${LB_BOOTSTRAP_QEMU_STATIC}" ]; then
			Echo_error "The qemu-static binary (${LB_BOOTSTRAP_QEMU_STATIC}) for ${LB_BOOTSTRAP_QEMU_ARCHITECTURE} is not executable on the host"
			exit 1
		fi
	fi
}

# Check option combinations and other extra stuff
Validate_config_dependencies ()
{
	if [ "${LB_BINARY_FILESYSTEM}" = "ntfs" ] && ! command -v ntfs-3g >/dev/null; then
		Echo_error "Using ntfs as the binary filesystem is currently only supported if ntfs-3g is installed on the host system."
		exit 1
	fi

	if [ "${LB_DEBIAN_INSTALLER}" != "none" ] && [ "${LB_DEBIAN_INSTALLER}" != "live" ]; then
		# d-i true, no caching
		if ! In_list "bootstrap" ${LB_CACHE_STAGES} || [ "${LB_CACHE}" != "true" ] || [ "${LB_CACHE_PACKAGES}" != "true" ]
		then
			Echo_warning "You have selected values of LB_CACHE, LB_CACHE_PACKAGES, LB_CACHE_STAGES and LB_DEBIAN_INSTALLER which will result in 'bootstrap' packages not being cached. This configuration is potentially unsafe as the bootstrap packages are re-used when integrating the Debian Installer."
		fi
	fi

	if In_list "syslinux" $LB_BOOTLOADERS; then
		# syslinux + fat or ntfs, or extlinux + ext[234] or btrfs
		if ! In_list "${LB_BINARY_FILESYSTEM}" fat16 fat32 ntfs ext2 ext3 ext4 btrfs; then
			Echo_warning "You have selected values of LB_BOOTLOADERS and LB_BINARY_FILESYSTEM which are incompatible - the syslinux family only support FAT, NTFS, ext[234] or btrfs filesystems."
		fi
	fi

	if In_list "grub-pc" ${LB_BOOTLOADERS} || In_list "grub-efi" ${LB_BOOTLOADERS} || In_list "grub-legacy" ${LB_BOOTLOADERS}; then
		if In_list "${LB_IMAGE_TYPE}" hdd netboot; then
			Echo_error "You have selected an invalid combination of bootloaders and live image type; the grub-* bootloaders are not compatible with hdd and netboot types."
			exit 1
		fi
	fi

	if [ "${LB_CHECKSUMS}" != "none" ] && [ "${LB_CHECKSUMS}" != "md5" ] && [ "${LB_INITRAMFS}" = "dracut-live" ]; then
		Echo_error "You have selected values of LB_CHECKSUMS and LB_INITRAMFS that are incompatible - dracut-live works only with no checksums or md5 checksums."
		exit 1
	fi

	Validate_http_proxy
}

# Retrieve the proxy settings from the host. Check whether conflicts are present with the command line arguments
Validate_http_proxy ()
{
	local HOST_AUTO_APT_PROXY=""
	local HOST_AUTO_APT_PROXY_LEGACY=""
	local HOST_FIXED_APT_PROXY=""

	# Fetch the proxy, using the various ways the http proxy can be set in apt
	if command -v apt-config >/dev/null; then
		local APT_CONFIG_OPTIONS
		# apt-config only understands --option (-o) and --config-file (-c) of ${APT_OPTIONS}
		# Don't report errors when additional options are provided and don't add additional quotes
		APT_CONFIG_OPTIONS=$(getopt --quiet --unquoted --options 'c:o:' --long 'config-file:,option:' -- ${APT_OPTIONS} || true)

		# The apt configuration `Acquire::http::Proxy-Auto-Detect` (and the legacy `Acquire::http::ProxyAutoDetect`)
		# If the script fails, or the result of the script is `DIRECT` or an empty line, it is considered to be not set (https://sources.debian.org/src/apt/2.3.9/apt-pkg/contrib/proxy.cc/)
		local AUTOPROXY
		eval "$(apt-config ${APT_CONFIG_OPTIONS} shell AUTOPROXY Acquire::http::Proxy-Auto-Detect)"
		if [ -x "${AUTOPROXY}" ]; then
			HOST_AUTO_APT_PROXY="$(${AUTOPROXY} || echo '')"
			if [ "${HOST_AUTO_APT_PROXY}" = "DIRECT" ]; then
				HOST_AUTO_APT_PROXY=""
			fi
		fi
		# Also check the legacy ProxyAutoDetect
		eval "$(apt-config ${APT_CONFIG_OPTIONS} shell AUTOPROXY Acquire::http::ProxyAutoDetect)"
		if [ -x "$AUTOPROXY" ]; then
			HOST_AUTO_APT_PROXY_LEGACY="$(${AUTOPROXY} || echo '')"
			if [ "${HOST_AUTO_APT_PROXY_LEGACY}" = "DIRECT" ]; then
				HOST_AUTO_APT_PROXY_LEGACY=""
			fi
		fi

		# The apt configuration `Acquire::http::proxy::URL-host` (https://sources.debian.org/src/apt/2.3.9/methods/http.cc/)
		# If set to `DIRECT`, it is considered to be not set
		#  This configuration allows you to specify different proxies for specific URLs
		#  This setup is too complex for the purpose of live-build and will silently be ignored

		# The apt configuration `Acquire::http::Proxy`
		eval "$(apt-config ${APT_CONFIG_OPTIONS} shell HOST_FIXED_APT_PROXY Acquire::http::Proxy)"
	fi


	# Report all detected settings in debug mode
	Echo_debug "Detected proxy settings:"
	Echo_debug "--apt-http-proxy: ${LB_APT_HTTP_PROXY}"
	Echo_debug "HOST Auto APT PROXY: ${HOST_AUTO_APT_PROXY}"
	Echo_debug "HOST Auto APT PROXY (legacy): ${HOST_AUTO_APT_PROXY_LEGACY}"
	Echo_debug "HOST Fixed APT PROXY: ${HOST_FIXED_APT_PROXY}"
	# The environment variable 'http_proxy' is used when no apt option is set
	Echo_debug "HOST http_proxy: ${http_proxy}"
	# The environment variable 'no_proxy' contains a list of domains that must not be handled by a proxy,
	# it overrides all previous settings by apt and 'http_proxy'
	Echo_debug "HOST no_proxy: ${no_proxy}"

	# Check whether any of the provided proxy values conflicts with another
	local LAST_SEEN_PROXY_NAME=""
	local LAST_SEEN_PROXY_VALUE=""
	Validate_http_proxy_source "apt configuration option Acquire::http::Proxy-Auto-Detect" "${HOST_AUTO_APT_PROXY}"
	Validate_http_proxy_source "apt configuration option Acquire::http::ProxyAutoDetect" "${HOST_AUTO_APT_PROXY_LEGACY}"
	Validate_http_proxy_source "apt configuration option Acquire::http::Proxy" "${HOST_FIXED_APT_PROXY}"
	Validate_http_proxy_source "environment variable http_proxy" "${http_proxy}"
	Validate_http_proxy_source "command line option --apt-http-proxy" "${LB_APT_HTTP_PROXY}"

	# This is the value to use for the other scripts in live-build
	export http_proxy="${LAST_SEEN_PROXY_VALUE}"
	if [ ! -z "${http_proxy}" ]; then
		Echo_message "Using http proxy: ${http_proxy}"
	fi
}

# Check whether a proxy setting conflicts with a previously set proxy setting
Validate_http_proxy_source ()
{
	local NAME="${1}"
	local VALUE="${2}"

	if [ ! -z "${VALUE}" ]; then
		if [ ! -z "${LAST_SEEN_PROXY_VALUE}" ]; then
			if [ "${VALUE}" != "${LAST_SEEN_PROXY_VALUE}" ]; then
				Echo_error "Inconsistent proxy configuration: the value for ${NAME} (${VALUE}) differs from the value for ${LAST_SEEN_PROXY_NAME} (${LAST_SEEN_PROXY_VALUE})"
				exit 1
			fi
		fi
		LAST_SEEN_PROXY_NAME="${NAME}"
		LAST_SEEN_PROXY_VALUE="${VALUE}"
	fi
}
