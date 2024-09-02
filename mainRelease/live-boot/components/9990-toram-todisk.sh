#!/bin/sh

#set -e

copy_live_to ()
{
	copyfrom="${1}"
	copytodev="${2}"
	copyto="${copyfrom}_swap"

	if [ -z "${MODULETORAM}" ]
	then
		size=$(fs_size "" ${copyfrom}/ "used")
	else
		MODULETORAMFILE="${copyfrom}/${LIVE_MEDIA_PATH}/${MODULETORAM}"

		if [ -f "${MODULETORAMFILE}" ]
		then
			size=$( expr $(ls -la ${MODULETORAMFILE} | awk '{print $5}') / 1024 + 5000 )
		else
			log_warning_msg "Error: toram-module ${MODULETORAM} (${MODULETORAMFILE}) could not be read."
			return 1
		fi
	fi

	if [ "${copytodev}" = "ram" ]
	then
		# copying to ram:
		freespace=$(awk '/^MemFree:/{f=$2} /^Cached:/{c=$2} END{print f+c}' /proc/meminfo)
		mount_options="-o size=${size}k"
		free_string="memory"
		fstype="tmpfs"
		dev="/dev/shm"
	else
		# it should be a writable block device
		if [ -b "${copytodev}" ]
		then
			dev="${copytodev}"
			free_string="space"
			fstype=$(get_fstype "${dev}")
			freespace=$(fs_size "${dev}")
		else
			log_warning_msg "${copytodev} is not a block device."
			return 1
		fi
	fi

	if [ "${freespace}" -lt "${size}" ]
	then
		log_warning_msg "Not enough free ${free_string} (${freespace}k free, ${size}k needed) to copy live media in ${copytodev}."
		return 1
	fi

	# Custom ramdisk size
	if [ -z "${mount_options}" ] && [ -n "${ramdisk_size}" ]
	then
		# FIXME: should check for wrong values
		mount_options="-o size=${ramdisk_size}"
	fi

	# begin copying (or uncompressing)
	mkdir "${copyto}"
	log_begin_msg "mount -t ${fstype} ${mount_options} ${dev} ${copyto}"
	mount -t "${fstype}" ${mount_options} "${dev}" "${copyto}"

	if [ "${extension}" = "tgz" ]
	then
		cd "${copyto}"
		tar zxf "${copyfrom}/${LIVE_MEDIA_PATH}/$(basename ${FETCH})"
		rm -f "${copyfrom}/${LIVE_MEDIA_PATH}/$(basename ${FETCH})"
		mount -r -o move "${copyto}" "${rootmnt}"
		cd "${OLDPWD}"
	else
		if [ -n "${MODULETORAMFILE}" ]
		then
			if [ -x /bin/rsync ]
			then
				echo " * Copying $MODULETORAMFILE to RAM" 1>/dev/console
				rsync -a --progress ${MODULETORAMFILE} ${copyto} 1>/dev/console # copy only the filesystem module
			else
				cp ${MODULETORAMFILE} ${copyto} # copy only the filesystem module
			fi
		else
			if [ -x /bin/rsync ]
			then
				echo " * Copying whole medium to RAM" 1>/dev/console
				rsync -a --progress ${copyfrom}/* ${copyto} 1>/dev/console  # "cp -a" from busybox also copies hidden files
			else
				cp -a ${copyfrom}/* ${copyto}/
				if [ -e ${copyfrom}/${LIVE_MEDIA_PATH}/.disk ]
				then
					cp -a ${copyfrom}/${LIVE_MEDIA_PATH}/.disk ${copyto}
				fi
			fi
		fi

		umount ${copyfrom}
		mount -r -o move ${copyto} ${copyfrom}
	fi

	rmdir ${copyto}
	return 0
}
