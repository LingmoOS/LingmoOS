#!/bin/sh

## live-build(7) - System Build Scripts
## Copyright (C) 2016-2020 The Debian Live team
## Copyright (C) 2006-2015 Daniel Baumann <mail@daniel-baumann.ch>
##
## This program comes with ABSOLUTELY NO WARRANTY; for details see COPYING.
## This is free software, and you are welcome to redistribute it
## under certain conditions; see COPYING for details.

Lodetach ()
{
	local DEVICE="${1}"
	local ATTEMPT="${2:-1}"

	if [ "${ATTEMPT}" -gt 3 ]
	then
		Echo_error "Failed to detach loop device '${DEVICE}'."
		exit 1
	fi

	# Changes to block devices result in uevents which trigger rules which in
	# turn access the loop device (ex. udisks-part-id, blkid) which can cause
	# a race condition. We call 'udevadm settle' to help avoid this.
	if command -v udevadm >/dev/null
	then
		udevadm settle
	fi

	# Loop back devices aren't the most reliable when it comes to writes.
	# We sleep and sync for good measure - better than build failure.
	sync
	sleep 1

	losetup -d "${DEVICE}" || Lodetach "${DEVICE}" "$(expr ${ATTEMPT} + 1)"
}

Losetup ()
{
	local DEVICE="${1}"
	local FILE="${2}"
	local PARTITION="${3:-1}"

	local FDISK_OUT
	local LOOPDEVICE
	losetup --read-only --partscan "${DEVICE}" "${FILE}"
	FDISK_OUT="$(fdisk -l -u ${DEVICE} 2>&1)"
	Lodetach "${DEVICE}"

	LOOPDEVICE="$(echo ${DEVICE}p${PARTITION})"

	if [ "${PARTITION}" = "0" ]
	then
		Echo_message "Mounting %s with offset 0" "${DEVICE}"

		losetup --partscan "${DEVICE}" "${FILE}"
	else
		local SECTORS
		local OFFSET
		SECTORS="$(echo "$FDISK_OUT" | sed -ne "s|^$LOOPDEVICE[ *]*\([0-9]*\).*|\1|p")"
		OFFSET="$(expr ${SECTORS} '*' 512)"

		Echo_message "Mounting %s with offset %s" "${DEVICE}" "${OFFSET}"

		losetup --partscan -o "${OFFSET}" "${DEVICE}" "${FILE}"
	fi
}

# adapted from lib/ext2fs/mkjournal.c, default block size is 4096 bytes (/etc/mke2fs.conf).
ext2fs_default_journal_size()
{
	local SIZE="$1"
	if [ "${SIZE}" -lt "8" ]; then # 2048*4096
		echo 0
	elif [ "${SIZE}" -lt "128" ]; then # 32768*4096
		echo 4
	elif [ "${SIZE}" -lt "1024" ]; then # 256*1024*4096
		echo 16
	elif [ "${SIZE}" -lt "2048" ]; then # 512*1024*4096
		echo 32
	elif [ "${SIZE}" -lt "4096" ]; then # 1024*1024*4096
		echo 64
	else
		echo 128
	fi
}

Calculate_partition_size_without_journal ()
{
	local WITHOUT_JOURNAL_ORIGINAL_SIZE="${1}"
	local WITHOUT_JOURNAL_FILESYSTEM="${2}"

	local PERCENT
	case "${WITHOUT_JOURNAL_FILESYSTEM}" in
		ext2|ext3|ext4)
			PERCENT="6"
			;;
		*)
			PERCENT="3"
			;;
	esac

	echo $(expr ${WITHOUT_JOURNAL_ORIGINAL_SIZE} + ${WITHOUT_JOURNAL_ORIGINAL_SIZE} \* ${PERCENT} / 100 + 1)
}

Calculate_partition_size ()
{
	local ORIGINAL_SIZE="${1}"
	local FILESYSTEM="${2}"

	case "${FILESYSTEM}" in
		ext3|ext4)
			local NON_JOURNAL_SIZE
			local PROJECTED_JOURNAL_SIZE
			local PROJECTED_PARTITION_SIZE
			local PRE_FINAL_PARTITION_SIZE
			local JOURNAL_SIZE
			NON_JOURNAL_SIZE=$(Calculate_partition_size_without_journal ${ORIGINAL_SIZE} ${FILESYSTEM})
			PROJECTED_JOURNAL_SIZE=$(ext2fs_default_journal_size ${NON_JOURNAL_SIZE})
			PROJECTED_PARTITION_SIZE=$(expr ${ORIGINAL_SIZE} + ${PROJECTED_JOURNAL_SIZE})
			PRE_FINAL_PARTITION_SIZE=$(Calculate_partition_size_without_journal ${PROJECTED_PARTITION_SIZE} ${FILESYSTEM})
			JOURNAL_SIZE=$(ext2fs_default_journal_size ${PRE_FINAL_PARTITION_SIZE})

			expr $(Calculate_partition_size_without_journal ${ORIGINAL_SIZE} ${FILESYSTEM}) + ${JOURNAL_SIZE}
			;;
		*)
			Calculate_partition_size_without_journal ${ORIGINAL_SIZE} ${FILESYSTEM}
			;;
	esac
}
