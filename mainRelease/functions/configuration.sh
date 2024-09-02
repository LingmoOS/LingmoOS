#!/bin/sh

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

	export LINGMO_SYS_BUILD_CONFIGURATION_VERSION="${LINGMO_SYS_BUILD_CONFIGURATION_VERSION:-${LIVE_BUILD_VERSION}}"
	export LIVE_CONFIGURATION_VERSION="${LINGMO_SYS_BUILD_CONFIGURATION_VERSION}" #for backwards compatibility with hooks

	LINGMO_SYS_BUILD_SYSTEM="${LINGMO_SYS_BUILD_SYSTEM:-live}"

	LINGMO_SYS_BUILD_MODE="${LINGMO_SYS_BUILD_MODE:-debian}"
	LINGMO_SYS_BUILD_DERIVATIVE="false"
	LINGMO_SYS_BUILD_DISTRIBUTION="${LINGMO_SYS_BUILD_DISTRIBUTION:-polaris}"
	LINGMO_SYS_BUILD_DISTRIBUTION_CHROOT="${LINGMO_SYS_BUILD_DISTRIBUTION_CHROOT:-${LINGMO_SYS_BUILD_DISTRIBUTION}}"
	LINGMO_SYS_BUILD_DISTRIBUTION_BINARY="${LINGMO_SYS_BUILD_DISTRIBUTION_BINARY:-${LINGMO_SYS_BUILD_DISTRIBUTION_CHROOT}}"

	# Do a reproducible build, i.e. is SOURCE_DATE_EPOCH already set?
	_REPRODUCIBLE="${SOURCE_DATE_EPOCH:-false}"
	if [ "${_REPRODUCIBLE}" != "false" ]; then
		_REPRODUCIBLE=true
	fi
	# For a reproducible build, use UTC per default, otherwise the local time
	_UTC_TIME_DEFAULT=${_REPRODUCIBLE}

	# The current time: for a unified timestamp throughout the building process
 	export SOURCE_DATE_EPOCH="${SOURCE_DATE_EPOCH:-$(date '+%s')}"

	LINGMO_SYS_BUILD_UTC_TIME="${LINGMO_SYS_BUILD_UTC_TIME:-${_UTC_TIME_DEFAULT}}"
	# Set UTC option for use with `date` where applicable
	if [ "${LINGMO_SYS_BUILD_UTC_TIME}" = "true" ]; then
		DATE_UTC_OPTION="--utc"
	else
		DATE_UTC_OPTION=""
	fi

	export LINGMO_SYS_BUILD_IMAGE_NAME="${LINGMO_SYS_BUILD_IMAGE_NAME:-live-image}"
	export LINGMO_SYS_BUILD_IMAGE_TYPE="${LINGMO_SYS_BUILD_IMAGE_TYPE:-iso-hybrid}"
	#for backwards compatibility with hooks
	export LIVE_IMAGE_NAME="${LINGMO_SYS_BUILD_IMAGE_NAME}"
	export LIVE_IMAGE_TYPE="${LINGMO_SYS_BUILD_IMAGE_TYPE}"

	if [ -z "${LINGMO_SYS_BUILD_ARCHITECTURE}" ]; then
		if command -v dpkg >/dev/null; then
			LINGMO_SYS_BUILD_ARCHITECTURE="$(dpkg --print-architecture)"
		else
			case "$(uname -m)" in
				x86_64)
					LINGMO_SYS_BUILD_ARCHITECTURE="amd64"
					;;

				i?86)
					LINGMO_SYS_BUILD_ARCHITECTURE="i386"
					;;

				*)
					Echo_error "Unable to determine current architecture"
					exit 1
					;;
			esac
		fi
	fi
	export LINGMO_SYS_BUILD_ARCHITECTURE
	# For backwards compat with custom hooks and conditional includes
	export LINGMO_SYS_BUILD_ARCHITECTURES="${LINGMO_SYS_BUILD_ARCHITECTURE}"

	LINGMO_SYS_BUILD_ARCHIVE_AREAS="${LINGMO_SYS_BUILD_ARCHIVE_AREAS:-main}"
	LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS="${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS:-${LINGMO_SYS_BUILD_ARCHIVE_AREAS}}"
	export LINGMO_SYS_BUILD_ARCHIVE_AREAS="$(echo "${LINGMO_SYS_BUILD_ARCHIVE_AREAS}" | tr "," " ")"
	export LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS="$(echo "${LINGMO_SYS_BUILD_PARENT_ARCHIVE_AREAS}" | tr "," " ")"

	LINGMO_SYS_BUILD_BACKPORTS="${LINGMO_SYS_BUILD_BACKPORTS:-false}"
	if [ -n "$LINGMO_SYS_BUILD_PARENT_DISTRIBUTION" ]; then
		LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_CHROOT="${LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_CHROOT:-${LINGMO_SYS_BUILD_PARENT_DISTRIBUTION}}"
		LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_BINARY="${LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_BINARY:-${LINGMO_SYS_BUILD_PARENT_DISTRIBUTION}}"
		LINGMO_SYS_BUILD_PARENT_DEBIAN_INSTALLER_DISTRIBUTION="${LINGMO_SYS_BUILD_PARENT_DEBIAN_INSTALLER_DISTRIBUTION:-${LINGMO_SYS_BUILD_PARENT_DISTRIBUTION}}"
	else
		LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_CHROOT="${LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_CHROOT:-${LINGMO_SYS_BUILD_DISTRIBUTION_CHROOT}}"
		LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_BINARY="${LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_BINARY:-${LINGMO_SYS_BUILD_DISTRIBUTION_BINARY}}"
		LINGMO_SYS_BUILD_PARENT_DEBIAN_INSTALLER_DISTRIBUTION="${LINGMO_SYS_BUILD_PARENT_DEBIAN_INSTALLER_DISTRIBUTION:-${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_DISTRIBUTION}}"
	fi

	LINGMO_SYS_BUILD_APT="${LINGMO_SYS_BUILD_APT:-apt}"
	LINGMO_SYS_BUILD_APT_HTTP_PROXY="${LINGMO_SYS_BUILD_APT_HTTP_PROXY}"
	LINGMO_SYS_BUILD_APT_RECOMMENDS="${LINGMO_SYS_BUILD_APT_RECOMMENDS:-true}"
	LINGMO_SYS_BUILD_APT_SECURE="${LINGMO_SYS_BUILD_APT_SECURE:-true}"
	LINGMO_SYS_BUILD_APT_SOURCE_ARCHIVES="${LINGMO_SYS_BUILD_APT_SOURCE_ARCHIVES:-true}"
	LINGMO_SYS_BUILD_APT_INDICES="${LINGMO_SYS_BUILD_APT_INDICES:-true}"

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

	LINGMO_SYS_BUILD_CACHE="${LINGMO_SYS_BUILD_CACHE:-true}"
	if [ "${LINGMO_SYS_BUILD_CACHE}" = "false" ]
	then
		LINGMO_SYS_BUILD_CACHE_INDICES="false"
		LINGMO_SYS_BUILD_CACHE_PACKAGES="false"
		LINGMO_SYS_BUILD_CACHE_STAGES="bootstrap" #bootstrap caching currently required for process to work
	else
		LINGMO_SYS_BUILD_CACHE_INDICES="${LINGMO_SYS_BUILD_CACHE_INDICES:-false}"
		LINGMO_SYS_BUILD_CACHE_PACKAGES="${LINGMO_SYS_BUILD_CACHE_PACKAGES:-true}"
		LINGMO_SYS_BUILD_CACHE_STAGES="${LINGMO_SYS_BUILD_CACHE_STAGES:-bootstrap}"
	fi
	LINGMO_SYS_BUILD_CACHE_STAGES="$(echo "${LINGMO_SYS_BUILD_CACHE_STAGES}" | tr "," " ")"

	LINGMO_SYS_BUILD_DEBCONF_FRONTEND="${LINGMO_SYS_BUILD_DEBCONF_FRONTEND:-noninteractive}"
	LINGMO_SYS_BUILD_DEBCONF_PRIORITY="${LINGMO_SYS_BUILD_DEBCONF_PRIORITY:-critical}"

	case "${LINGMO_SYS_BUILD_SYSTEM}" in
		live)
			LINGMO_SYS_BUILD_INITRAMFS="${LINGMO_SYS_BUILD_INITRAMFS:-live-boot}"
			;;

		normal)
			LINGMO_SYS_BUILD_INITRAMFS="${LINGMO_SYS_BUILD_INITRAMFS:-none}"
			;;
	esac

	LINGMO_SYS_BUILD_INITRAMFS_COMPRESSION="${LINGMO_SYS_BUILD_INITRAMFS_COMPRESSION:-gzip}"

	case "${LINGMO_SYS_BUILD_SYSTEM}" in
		live)
			LINGMO_SYS_BUILD_INITSYSTEM="${LINGMO_SYS_BUILD_INITSYSTEM:-systemd}"
			;;

		normal)
			LINGMO_SYS_BUILD_INITSYSTEM="${LINGMO_SYS_BUILD_INITSYSTEM:-none}"
			;;
	esac

	if [ "${LINGMO_SYS_BUILD_ARCHITECTURE}" = "i386" ] && [ "${CURRENT_IMAGE_ARCHITECTURE}" = "amd64" ]
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
	if [ "${LINGMO_SYS_BUILD_MODE}" = "debian" ]; then
		LINGMO_SYS_BUILD_MIRROR_BOOTSTRAP="${LINGMO_SYS_BUILD_MIRROR_BOOTSTRAP:-https://packages.lingmo.org}"
		LINGMO_SYS_BUILD_PARENT_MIRROR_BOOTSTRAP="${LINGMO_SYS_BUILD_PARENT_MIRROR_BOOTSTRAP:-${LINGMO_SYS_BUILD_MIRROR_BOOTSTRAP}}"
	fi
	LINGMO_SYS_BUILD_MIRROR_CHROOT="${LINGMO_SYS_BUILD_MIRROR_CHROOT:-${LINGMO_SYS_BUILD_MIRROR_BOOTSTRAP}}"
	LINGMO_SYS_BUILD_PARENT_MIRROR_CHROOT="${LINGMO_SYS_BUILD_PARENT_MIRROR_CHROOT:-${LINGMO_SYS_BUILD_PARENT_MIRROR_BOOTSTRAP}}"
	if [ "${LINGMO_SYS_BUILD_MODE}" = "debian" ]; then
		LINGMO_SYS_BUILD_MIRROR_CHROOT_SECURITY="${LINGMO_SYS_BUILD_MIRROR_CHROOT_SECURITY:-http://security.debian.org/}"
		LINGMO_SYS_BUILD_PARENT_MIRROR_CHROOT_SECURITY="${LINGMO_SYS_BUILD_PARENT_MIRROR_CHROOT_SECURITY:-${LINGMO_SYS_BUILD_MIRROR_CHROOT_SECURITY}}"

		LINGMO_SYS_BUILD_MIRROR_BINARY="${LINGMO_SYS_BUILD_MIRROR_BINARY:-http://packages.lingmo.org}"
		LINGMO_SYS_BUILD_PARENT_MIRROR_BINARY="${LINGMO_SYS_BUILD_PARENT_MIRROR_BINARY:-${LINGMO_SYS_BUILD_MIRROR_BINARY}}"

		LINGMO_SYS_BUILD_MIRROR_BINARY_SECURITY="${LINGMO_SYS_BUILD_MIRROR_BINARY_SECURITY:-http://security.debian.org/}"
		LINGMO_SYS_BUILD_PARENT_MIRROR_BINARY_SECURITY="${LINGMO_SYS_BUILD_PARENT_MIRROR_BINARY_SECURITY:-${LINGMO_SYS_BUILD_MIRROR_BINARY_SECURITY}}"
	fi
	LINGMO_SYS_BUILD_MIRROR_DEBIAN_INSTALLER="${LINGMO_SYS_BUILD_MIRROR_DEBIAN_INSTALLER:-${LINGMO_SYS_BUILD_MIRROR_CHROOT}}"
	LINGMO_SYS_BUILD_PARENT_MIRROR_DEBIAN_INSTALLER="${LINGMO_SYS_BUILD_PARENT_MIRROR_DEBIAN_INSTALLER:-${LINGMO_SYS_BUILD_PARENT_MIRROR_CHROOT}}"

	LINGMO_SYS_BUILD_CHROOT_FILESYSTEM="${LINGMO_SYS_BUILD_CHROOT_FILESYSTEM:-squashfs}"

	LINGMO_SYS_BUILD_UNION_FILESYSTEM="${LINGMO_SYS_BUILD_UNION_FILESYSTEM:-overlay}"

	LINGMO_SYS_BUILD_INTERACTIVE="${LINGMO_SYS_BUILD_INTERACTIVE:-false}"

	LINGMO_SYS_BUILD_KEYRING_PACKAGES="${LINGMO_SYS_BUILD_KEYRING_PACKAGES:-debian-archive-keyring}"

	# first, handle existing LINGMO_SYS_BUILD_LINUX_FLAVOURS for backwards compatibility
	if [ -n "${LINGMO_SYS_BUILD_LINUX_FLAVOURS}" ]; then
		LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH="${LINGMO_SYS_BUILD_LINUX_FLAVOURS}"
	fi
	case "${LINGMO_SYS_BUILD_ARCHITECTURE}" in
		arm64)
			LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH="${LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH:-arm64}"
			;;

		armel)
			# armel will have special images: one rootfs image and many additional kernel images.
			# therefore we default to all available armel flavours
			LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH="${LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH:-marvell}"
			;;

		armhf)
			# armhf will have special images: one rootfs image and many additional kernel images.
			# therefore we default to all available armhf flavours
			LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH="${LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH:-armmp armmp-lpae}"
			;;

		amd64)
			LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH="${LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH:-amd64}"
			;;

		i386)
			LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH="${LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH:-686-pae}"
			;;

		ia64)
			LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH="${LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH:-itanium}"
			;;

		powerpc)
			LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH="${LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH:-powerpc64 powerpc}"
			;;

		ppc64el)
			LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH="${LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH:-powerpc64le}"
			;;

		riscv64)
			LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH="${LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH:-riscv64}"
			;;

		s390x)
			LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH="${LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH:-s390x}"
			;;

		*)
			Echo_error "Architecture(s) ${LINGMO_SYS_BUILD_ARCHITECTURE} not yet supported (FIXME)"
			exit 1
			;;
	esac

	LINGMO_SYS_BUILD_LINUX_FLAVOURS=""
	for FLAVOUR in ${LINGMO_SYS_BUILD_LINUX_FLAVOURS_WITH_ARCH}
	do
		ARCH_FILTERED_FLAVOUR="$(echo ${FLAVOUR} | awk -F':' '{print $1}')"
		LINGMO_SYS_BUILD_LINUX_FLAVOURS="${LINGMO_SYS_BUILD_LINUX_FLAVOURS:+$LINGMO_SYS_BUILD_LINUX_FLAVOURS }${ARCH_FILTERED_FLAVOUR}"
	done

	LINGMO_SYS_BUILD_LINUX_PACKAGES="${LINGMO_SYS_BUILD_LINUX_PACKAGES:-linux-image}"

	LINGMO_SYS_BUILD_BINARY_FILESYSTEM="${LINGMO_SYS_BUILD_BINARY_FILESYSTEM:-fat32}"

	case "${LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_BINARY}" in
		sid|unstable)
			LINGMO_SYS_BUILD_SECURITY="${LINGMO_SYS_BUILD_SECURITY:-false}"
			;;

		*)
			LINGMO_SYS_BUILD_SECURITY="${LINGMO_SYS_BUILD_SECURITY:-true}"
			;;
	esac

	case "${LINGMO_SYS_BUILD_PARENT_DISTRIBUTION_BINARY}" in
		sid|unstable)
			LINGMO_SYS_BUILD_UPDATES="${LINGMO_SYS_BUILD_UPDATES:-false}"
			;;

		*)
			LINGMO_SYS_BUILD_UPDATES="${LINGMO_SYS_BUILD_UPDATES:-true}"
			;;
	esac

	case "${LINGMO_SYS_BUILD_ARCHITECTURE}" in
		amd64|i386)
			LINGMO_SYS_BUILD_IMAGE_TYPE="${LINGMO_SYS_BUILD_IMAGE_TYPE:-iso-hybrid}"
			;;

		*)
			LINGMO_SYS_BUILD_IMAGE_TYPE="${LINGMO_SYS_BUILD_IMAGE_TYPE:-iso}"
			;;
	esac

	case "${LINGMO_SYS_BUILD_ARCHITECTURE}" in
		amd64|i386)
			LINGMO_SYS_BUILD_BOOTLOADER_BIOS="${LINGMO_SYS_BUILD_BOOTLOADER_BIOS:-syslinux}"
			if ! In_list "${LINGMO_SYS_BUILD_IMAGE_TYPE}" hdd netboot; then
				LINGMO_SYS_BUILD_BOOTLOADER_EFI="${LINGMO_SYS_BUILD_BOOTLOADER_EFI:-grub-efi}"
			fi
			;;
	esac
	# Command line option combines BIOS and EFI selection in one.
	# Also, need to support old config files that held `LINGMO_SYS_BUILD_BOOTLOADERS`.
	# Note that this function does not perform validation, so none is done here.
	if [ -n "${LINGMO_SYS_BUILD_BOOTLOADERS}" ]; then
		LINGMO_SYS_BUILD_BOOTLOADERS="$(echo "${LINGMO_SYS_BUILD_BOOTLOADERS}" | tr "," " ")"
		unset LINGMO_SYS_BUILD_BOOTLOADER_BIOS
		unset LINGMO_SYS_BUILD_BOOTLOADER_EFI
		local BOOTLOADER
		for BOOTLOADER in $LINGMO_SYS_BUILD_BOOTLOADERS; do
			case "${BOOTLOADER}" in
				grub-legacy|grub-pc|syslinux)
					LINGMO_SYS_BUILD_BOOTLOADER_BIOS="${BOOTLOADER}"
					;;
				grub-efi)
					LINGMO_SYS_BUILD_BOOTLOADER_EFI="${BOOTLOADER}"
					;;
			esac
		done
	fi

	LINGMO_SYS_BUILD_CHECKSUMS="${LINGMO_SYS_BUILD_CHECKSUMS:-sha256}"

	LINGMO_SYS_BUILD_COMPRESSION="${LINGMO_SYS_BUILD_COMPRESSION:-none}"

	LINGMO_SYS_BUILD_ZSYNC="${LINGMO_SYS_BUILD_ZSYNC:-true}"

	LINGMO_SYS_BUILD_BUILD_WITH_CHROOT="${LINGMO_SYS_BUILD_BUILD_WITH_CHROOT:-true}"

	LINGMO_SYS_BUILD_BUILD_WITH_TMPFS="${LINGMO_SYS_BUILD_BUILD_WITH_TMPFS:-false}"

	LINGMO_SYS_BUILD_DEBIAN_INSTALLER="${LINGMO_SYS_BUILD_DEBIAN_INSTALLER:-none}"
	if [ "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER}" = "false" ]
	then
		LINGMO_SYS_BUILD_DEBIAN_INSTALLER="none"
		Echo_warning "A value of 'false' for option LINGMO_SYS_BUILD_DEBIAN_INSTALLER is deprecated, please use 'none' in future."
	fi
	if [ "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER}" = "true" ]
	then
		LINGMO_SYS_BUILD_DEBIAN_INSTALLER="netinst"
		Echo_warning "A value of 'true' for option LINGMO_SYS_BUILD_DEBIAN_INSTALLER is deprecated, please use 'netinst' in future."
	fi

	# cdrom-checker in d-i requires a md5 checksum file
	if [ "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER}" != "none" ]
	then
		if [ "${LINGMO_SYS_BUILD_CHECKSUMS}" = "none" ]
		then
			LINGMO_SYS_BUILD_CHECKSUMS="md5"
		else
			if ! In_list md5 ${LINGMO_SYS_BUILD_CHECKSUMS}; then
				LINGMO_SYS_BUILD_CHECKSUMS=${LINGMO_SYS_BUILD_CHECKSUMS}" md5"
			fi
		fi
	fi

	LINGMO_SYS_BUILD_DEBIAN_INSTALLER_DISTRIBUTION="${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_DISTRIBUTION:-${LINGMO_SYS_BUILD_DISTRIBUTION}}"
	LINGMO_SYS_BUILD_DEBIAN_INSTALLER_GUI="${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_GUI:-true}"

	if [ -z "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_PRESEEDFILE}" ]
	then
		if Find_files config/debian-installer/preseed.cfg
		then
			LINGMO_SYS_BUILD_DEBIAN_INSTALLER_PRESEEDFILE="/preseed.cfg"
		fi

		if Find_files config/debian-installer/*.cfg && [ ! -e config/debian-installer/preseed.cfg ]
		then
			Echo_warning "You have placed some preseeding files into config/debian-installer but you didn't specify the default preseeding file through LINGMO_SYS_BUILD_DEBIAN_INSTALLER_PRESEEDFILE. This means that debian-installer will not take up a preseeding file by default."
		fi
	fi

	case "${LINGMO_SYS_BUILD_INITRAMFS}" in
		live-boot)
			LINGMO_SYS_BUILD_BOOTAPPEND_LIVE="${LINGMO_SYS_BUILD_BOOTAPPEND_LIVE:-boot=live components quiet splash}"
			LINGMO_SYS_BUILD_BOOTAPPEND_LIVE_FAILSAFE="${LINGMO_SYS_BUILD_BOOTAPPEND_LIVE_FAILSAFE:-boot=live components memtest noapic noapm nodma nomce nolapic nosmp nosplash vga=788}"
			;;

		none)
			LINGMO_SYS_BUILD_BOOTAPPEND_LIVE="${LINGMO_SYS_BUILD_BOOTAPPEND_LIVE:-quiet splash}"
			LINGMO_SYS_BUILD_BOOTAPPEND_LIVE_FAILSAFE="${LINGMO_SYS_BUILD_BOOTAPPEND_LIVE_FAILSAFE:-memtest noapic noapm nodma nomce nolapic nosmp nosplash vga=788}"
			;;
	esac

	local _LINGMO_SYS_BUILD_BOOTAPPEND_PRESEED
	if [ -n "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_PRESEEDFILE}" ]
	then
		case "${LINGMO_SYS_BUILD_IMAGE_TYPE}" in
			iso|iso-hybrid)
				_LINGMO_SYS_BUILD_BOOTAPPEND_PRESEED="file=/cdrom/install/${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_PRESEEDFILE}"
				;;

			hdd)
				_LINGMO_SYS_BUILD_BOOTAPPEND_PRESEED="file=/hd-media/install/${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_PRESEEDFILE}"
				;;

			netboot)
				case "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_PRESEEDFILE}" in
					*://*)
						_LINGMO_SYS_BUILD_BOOTAPPEND_PRESEED="file=${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_PRESEEDFILE}"
						;;

					*)
						_LINGMO_SYS_BUILD_BOOTAPPEND_PRESEED="file=/${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_PRESEEDFILE}"
						;;
				esac
				;;

			tar)
				;;
		esac
	fi

	if [ -n ${_LINGMO_SYS_BUILD_BOOTAPPEND_PRESEED} ]
	then
		LINGMO_SYS_BUILD_BOOTAPPEND_INSTALL="${LINGMO_SYS_BUILD_BOOTAPPEND_INSTALL} ${_LINGMO_SYS_BUILD_BOOTAPPEND_PRESEED}"
	fi

	LINGMO_SYS_BUILD_BOOTAPPEND_INSTALL="$(echo ${LINGMO_SYS_BUILD_BOOTAPPEND_INSTALL} | sed -e 's/[ \t]*$//')"

	LINGMO_SYS_BUILD_ISO_APPLICATION="${LINGMO_SYS_BUILD_ISO_APPLICATION:-Lingmo OS Live}"
	LINGMO_SYS_BUILD_ISO_PREPARER="${LINGMO_SYS_BUILD_ISO_PREPARER:-lingmo-system-build @LINGMO_SYS_BUILD_VERSION@; https://github.com/LingmoOS/lingmo-system-build}"
	LINGMO_SYS_BUILD_ISO_PUBLISHER="${LINGMO_SYS_BUILD_ISO_PUBLISHER:-Lingmo OS project; https://github.com/LingmoOS/lingmo-system-build; team@lingmo.org}"
	# The string @ISOVOLUME_TS@ must have the same length as the output of `date +%Y%m%d-%H:%M`
	LINGMO_SYS_BUILD_ISO_VOLUME="${LINGMO_SYS_BUILD_ISO_VOLUME:-Debian ${LINGMO_SYS_BUILD_DISTRIBUTION} @ISOVOLUME_TS@}"

	LINGMO_SYS_BUILD_HDD_LABEL="${LINGMO_SYS_BUILD_HDD_LABEL:-DEBIAN_LIVE}"
	LINGMO_SYS_BUILD_HDD_SIZE="${LINGMO_SYS_BUILD_HDD_SIZE:-auto}"

	LINGMO_SYS_BUILD_MEMTEST="${LINGMO_SYS_BUILD_MEMTEST:-none}"
	if [ "${LINGMO_SYS_BUILD_MEMTEST}" = "false" ]; then
		LINGMO_SYS_BUILD_MEMTEST="none"
		Echo_warning "A value of 'false' for option LINGMO_SYS_BUILD_MEMTEST is deprecated, please use 'none' in future."
	fi

	case "${LINGMO_SYS_BUILD_ARCHITECTURE}" in
		amd64|i386)
			if [ "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER}" != "none" ]; then
				LINGMO_SYS_BUILD_LOADLIN="${LINGMO_SYS_BUILD_LOADLIN:-true}"
			else
				LINGMO_SYS_BUILD_LOADLIN="${LINGMO_SYS_BUILD_LOADLIN:-false}"
			fi
			;;

		*)
			LINGMO_SYS_BUILD_LOADLIN="${LINGMO_SYS_BUILD_LOADLIN:-false}"
			;;
	esac

	LINGMO_SYS_BUILD_WIN32_LOADER="${LINGMO_SYS_BUILD_WIN32_LOADER:-false}"

	LINGMO_SYS_BUILD_NET_TARBALL="${LINGMO_SYS_BUILD_NET_TARBALL:-true}"

	LINGMO_SYS_BUILD_ONIE="${LINGMO_SYS_BUILD_ONIE:-false}"
	LINGMO_SYS_BUILD_ONIE_KERNEL_CMDLINE="${LINGMO_SYS_BUILD_ONIE_KERNEL_CMDLINE:-}"

	LINGMO_SYS_BUILD_FIRMWARE_CHROOT="${LINGMO_SYS_BUILD_FIRMWARE_CHROOT:-true}"
	LINGMO_SYS_BUILD_FIRMWARE_BINARY="${LINGMO_SYS_BUILD_FIRMWARE_BINARY:-true}"

	LINGMO_SYS_BUILD_SWAP_FILE_SIZE="${LINGMO_SYS_BUILD_SWAP_FILE_SIZE:-512}"

	LINGMO_SYS_BUILD_UEFI_SECURE_BOOT="${LINGMO_SYS_BUILD_UEFI_SECURE_BOOT:-auto}"

	LINGMO_SYS_BUILD_SOURCE="${LINGMO_SYS_BUILD_SOURCE:-false}"
	LINGMO_SYS_BUILD_SOURCE_IMAGES="${LINGMO_SYS_BUILD_SOURCE_IMAGES:-tar}"
	LINGMO_SYS_BUILD_SOURCE_IMAGES="$(echo "${LINGMO_SYS_BUILD_SOURCE_IMAGES}" | tr "," " ")"

	# Foreign/port bootstrapping
	if [ -n "${LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_ARCHITECTURES}" ]; then
		LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_ARCHITECTURE="${LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_ARCHITECTURES}"
		unset LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_ARCHITECTURES
		Echo_warning "LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_ARCHITECTURES was renamed to LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_ARCHITECTURE, please updated your config."
	fi
	LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_ARCHITECTURE="${LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_ARCHITECTURE:-}"
	LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_EXCLUDE="${LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_EXCLUDE:-}"
	LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_STATIC="${LINGMO_SYS_BUILD_BOOTSTRAP_QEMU_STATIC:-}"
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
	if [ "${LINGMO_SYS_BUILD_APT_INDICES}" != "true" ] && [ "${LINGMO_SYS_BUILD_APT_INDICES}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_APT_INDICES (--apt-indices) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_APT_RECOMMENDS}" != "true" ] && [ "${LINGMO_SYS_BUILD_APT_RECOMMENDS}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_APT_RECOMMENDS (--apt-recommends) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_APT_SECURE}" != "true" ] && [ "${LINGMO_SYS_BUILD_APT_SECURE}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_APT_SECURE (--apt-secure) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_APT_SOURCE_ARCHIVES}" != "true" ] && [ "${LINGMO_SYS_BUILD_APT_SOURCE_ARCHIVES}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_APT_SOURCE_ARCHIVES (--apt-source-archives) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_BACKPORTS}" != "true" ] && [ "${LINGMO_SYS_BUILD_BACKPORTS}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_BACKPORTS (--backports) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_BUILD_WITH_CHROOT}" != "true" ] && [ "${LINGMO_SYS_BUILD_BUILD_WITH_CHROOT}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_BUILD_WITH_CHROOT (--build-with-chroot) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_CACHE}" != "true" ] && [ "${LINGMO_SYS_BUILD_CACHE}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_CACHE (--cache) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_CACHE_INDICES}" != "true" ] && [ "${LINGMO_SYS_BUILD_CACHE_INDICES}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_CACHE_INDICES (--cache-indices) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_CACHE_PACKAGES}" != "true" ] && [ "${LINGMO_SYS_BUILD_CACHE_PACKAGES}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_CACHE_PACKAGES (--cache-packages) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_GUI}" != "true" ] && [ "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER_GUI}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_DEBIAN_INSTALLER_GUI (--debian-installer-gui) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_FIRMWARE_BINARY}" != "true" ] && [ "${LINGMO_SYS_BUILD_FIRMWARE_BINARY}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_FIRMWARE_BINARY (--firmware-binary) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_FIRMWARE_CHROOT}" != "true" ] && [ "${LINGMO_SYS_BUILD_FIRMWARE_CHROOT}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_FIRMWARE_CHROOT (--firmware-chroot) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_LOADLIN}" != "true" ] && [ "${LINGMO_SYS_BUILD_LOADLIN}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_LOADLIN (--loadlin) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_NET_TARBALL}" != "true" ] && [ "${LINGMO_SYS_BUILD_NET_TARBALL}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_NET_TARBALL (--net-tarball) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_ONIE}" != "true" ] && [ "${LINGMO_SYS_BUILD_ONIE}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_ONIE (--onie) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_SECURITY}" != "true" ] && [ "${LINGMO_SYS_BUILD_SECURITY}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_SECURITY (--security) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_SOURCE}" != "true" ] && [ "${LINGMO_SYS_BUILD_SOURCE}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_SOURCE (--source) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_UPDATES}" != "true" ] && [ "${LINGMO_SYS_BUILD_UPDATES}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_UPDATES (--updates) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_UTC_TIME}" != "true" ] && [ "${LINGMO_SYS_BUILD_UTC_TIME}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_UTC_TIME (--utc-time) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_WIN32_LOADER}" != "true" ] && [ "${LINGMO_SYS_BUILD_WIN32_LOADER}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_WIN32_LOADER (--win32-loader) can only be 'true' or 'false'!"
		exit 1
	fi
	if [ "${LINGMO_SYS_BUILD_ZSYNC}" != "true" ] && [ "${LINGMO_SYS_BUILD_ZSYNC}" != "false" ]; then
		Echo_error "Value for LINGMO_SYS_BUILD_ZSYNC (--zsync) can only be 'true' or 'false'!"
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_APT}" apt apt-get aptitude; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_APT (--apt)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_BINARY_FILESYSTEM}" fat16 fat32 ext2 ext3 ext4 ntfs; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_BINARY_FILESYSTEM (--binary-filesystem)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_IMAGE_TYPE}" iso iso-hybrid hdd tar netboot; then
		Echo_error "You have specified an invalid value for --binary-image."
		exit 1
	fi

	if [ -z "${LINGMO_SYS_BUILD_BOOTLOADER_BIOS}" ] && [ -z "${LINGMO_SYS_BUILD_BOOTLOADER_EFI}" ]; then
		Echo_warning "You have specified no bootloaders; I predict that you will experience some problems!"
	fi
	if [ -n "${LINGMO_SYS_BUILD_BOOTLOADER_BIOS}" ] && ! In_list "${LINGMO_SYS_BUILD_BOOTLOADER_BIOS}" grub-legacy grub-pc syslinux; then
		Echo_error "You have specified an invalid BIOS bootloader."
		exit 1
	fi
	if [ -n "${LINGMO_SYS_BUILD_BOOTLOADER_EFI}" ] && ! In_list "${LINGMO_SYS_BUILD_BOOTLOADER_EFI}" grub-efi; then
		Echo_error "You have specified an invalid EFI bootloader."
		exit 1
	fi
	if [ -n "${LINGMO_SYS_BUILD_BOOTLOADERS}" ]; then
		local BOOTLOADER
		local BOOTLOADERS_BIOS=0
		local BOOTLOADERS_EFI=0
		for BOOTLOADER in $LINGMO_SYS_BUILD_BOOTLOADERS; do
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
	for CACHE_STAGE in ${LINGMO_SYS_BUILD_CACHE_STAGES}; do
		if ! In_list "${CACHE_STAGE}" bootstrap chroot rootfs; then
			Echo_warning "The following is not a valid stage: '%s'" "${CACHE_STAGE}"
		fi
	done

	local CHECKSUM
	if [ "${LINGMO_SYS_BUILD_CHECKSUMS}" != "none" ]; then
		for CHECKSUM in ${LINGMO_SYS_BUILD_CHECKSUMS}; do
			if ! In_list "${CHECKSUM}" md5 sha1 sha224 sha256 sha384 sha512; then
				Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_CHECKSUMS (--checksums): '%s'" "${CHECKSUM}"
				exit 1
			fi
		done
	fi

	if ! In_list "${LINGMO_SYS_BUILD_CHROOT_FILESYSTEM}" ext2 ext3 ext4 squashfs jffs2 none plain; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_CHROOT_FILESYSTEM (--chroot-filesystem)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_COMPRESSION}" bzip2 gzip lzip xz none; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_COMPRESSION (--compression)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_DEBCONF_FRONTEND}" dialog editor noninteractive readline; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_DEBCONF_FRONTEND (--debconf-frontend)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_DEBCONF_PRIORITY}" low medium high critical; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_DEBCONF_PRIORITY (--debconf-priority)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER}" cdrom netinst netboot businesscard live none; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_DEBIAN_INSTALLER (--debian-installer)."
		exit 1
	fi

	if echo ${LINGMO_SYS_BUILD_HDD_LABEL} | grep -qs ' '; then
		Echo_error "Whitespace is not currently supported in HDD labels (LINGMO_SYS_BUILD_HDD_LABEL; --hdd-label)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_INITRAMFS}" none live-boot; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_INITRAMFS (--initramfs)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_INITRAMFS_COMPRESSION}" bzip2 gzip lzma; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_INITRAMFS_COMPRESSION (--initramfs-compression)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_INITSYSTEM}" sysvinit systemd none; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_INITSYSTEM (--initsystem)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_INTERACTIVE}" true shell x11 xnest false; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_INTERACTIVE (--interactive)."
		exit 1
	fi

	if [ "$(echo -n "${LINGMO_SYS_BUILD_ISO_APPLICATION}" | wc -c)" -gt 128 ]; then
		Echo_warning "You have specified a value of LINGMO_SYS_BUILD_ISO_APPLICATION (--iso-application) that is too long; the maximum length is 128 characters."
	fi

	if [ "$(echo -n "${LINGMO_SYS_BUILD_ISO_PREPARER}" | sed -e "s/@LINGMO_SYS_BUILD_VERSION@/${VERSION}/" | wc -c)" -gt  128 ]; then
		Echo_warning "You have specified a value of LINGMO_SYS_BUILD_ISO_PREPARER (--iso-preparer) that is too long; the maximum length is 128 characters."
	fi

	if [ "$(echo -n "${LINGMO_SYS_BUILD_ISO_PUBLISHER}" | wc -c)" -gt 128 ]; then
		Echo_warning "You have specified a value of LINGMO_SYS_BUILD_ISO_PUBLISHER (--iso-publisher) that is too long; the maximum length is 128 characters."
	fi

	if [ "$(echo -n "${LINGMO_SYS_BUILD_ISO_VOLUME}" | sed -e "s/@ISOVOLUME_TS@/$(date $DATE_UTC_OPTION -d@${SOURCE_DATE_EPOCH} +%Y%m%d-%H:%M)/" | wc -c)" -gt 32 ]; then
		Echo_warning "You have specified a value of LINGMO_SYS_BUILD_ISO_VOLUME (--iso-volume) that is too long; the maximum length is 32 characters."
	fi

	if ! In_list "${LINGMO_SYS_BUILD_MEMTEST}" memtest86+ memtest86 none; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_MEMTEST (--memtest)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_SOURCE_IMAGES}" iso netboot tar hdd; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_SOURCE_IMAGES (--source-images)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_SYSTEM}" live normal; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_SYSTEM (--system)."
		exit 1
	fi

	if ! In_list "${LINGMO_SYS_BUILD_UEFI_SECURE_BOOT}" auto enable disable; then
		Echo_error "You have specified an invalid value for LINGMO_SYS_BUILD_UEFI_SECURE_BOOT (--uefi-secure-boot)."
		exit 1
	fi
}

# Check option combinations and other extra stuff
Validate_config_dependencies ()
{
	if [ "${LINGMO_SYS_BUILD_BINARY_FILESYSTEM}" = "ntfs" ] && ! command -v ntfs-3g >/dev/null; then
		Echo_error "Using ntfs as the binary filesystem is currently only supported if ntfs-3g is installed on the host system."
		exit 1
	fi

	if [ "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER}" != "none" ] && [ "${LINGMO_SYS_BUILD_DEBIAN_INSTALLER}" != "live" ]; then
		# d-i true, no caching
		if ! In_list "bootstrap" ${LINGMO_SYS_BUILD_CACHE_STAGES} || [ "${LINGMO_SYS_BUILD_CACHE}" != "true" ] || [ "${LINGMO_SYS_BUILD_CACHE_PACKAGES}" != "true" ]
		then
			Echo_warning "You have selected values of LINGMO_SYS_BUILD_CACHE, LINGMO_SYS_BUILD_CACHE_PACKAGES, LINGMO_SYS_BUILD_CACHE_STAGES and LINGMO_SYS_BUILD_DEBIAN_INSTALLER which will result in 'bootstrap' packages not being cached. This configuration is potentially unsafe as the bootstrap packages are re-used when integrating the Debian Installer."
		fi
	fi

	if In_list "syslinux" $LINGMO_SYS_BUILD_BOOTLOADERS; then
		# syslinux + fat or ntfs, or extlinux + ext[234] or btrfs
		if ! In_list "${LINGMO_SYS_BUILD_BINARY_FILESYSTEM}" fat16 fat32 ntfs ext2 ext3 ext4 btrfs; then
			Echo_warning "You have selected values of LINGMO_SYS_BUILD_BOOTLOADERS and LINGMO_SYS_BUILD_BINARY_FILESYSTEM which are incompatible - the syslinux family only support FAT, NTFS, ext[234] or btrfs filesystems."
		fi
	fi

	if In_list "grub-pc" ${LINGMO_SYS_BUILD_BOOTLOADERS} || In_list "grub-efi" ${LINGMO_SYS_BUILD_BOOTLOADERS} || In_list "grub-legacy" ${LINGMO_SYS_BUILD_BOOTLOADERS}; then
		if In_list "${LINGMO_SYS_BUILD_IMAGE_TYPE}" hdd netboot; then
			Echo_error "You have selected an invalid combination of bootloaders and live image type; the grub-* bootloaders are not compatible with hdd and netboot types."
			exit 1
		fi
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
		#  This setup is too complex for the purpose of lingmo-system-build and will silently be ignored

		# The apt configuration `Acquire::http::Proxy`
		eval "$(apt-config ${APT_CONFIG_OPTIONS} shell HOST_FIXED_APT_PROXY Acquire::http::Proxy)"
	fi


	# Report all detected settings in debug mode
	Echo_debug "Detected proxy settings:"
	Echo_debug "--apt-http-proxy: ${LINGMO_SYS_BUILD_APT_HTTP_PROXY}"
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
	Validate_http_proxy_source "command line option --apt-http-proxy" "${LINGMO_SYS_BUILD_APT_HTTP_PROXY}"

	# This is the value to use for the the other scripts in lingmo-system-build
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
