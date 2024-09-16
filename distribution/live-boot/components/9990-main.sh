#!/bin/sh

# set -e

Live ()
{
	if [ -x /scripts/local-top/cryptroot ]
	then
		/scripts/local-top/cryptroot
	fi

	exec 6>&1
	exec 7>&2
	exec > boot.log
	exec 2>&1
	tail -f boot.log >&7 &
	tailpid="${!}"

	LIVE_BOOT_CMDLINE="${LIVE_BOOT_CMDLINE:-$(cat /proc/cmdline)}"
	Cmdline_old

	Debug

	Read_only

	Select_eth_device

	if [ -e /conf/param.conf ]
	then
		. /conf/param.conf
	fi

	# Needed here too because some things (*cough* udev *cough*)
	# changes the timeout

	if [ -n "${NETBOOT}" ] || [ -n "${FETCH}" ] || [ -n "${HTTPFS}" ] || [ -n "${FTPFS}" ]
	then
		if do_netmount
		then
			livefs_root="${mountpoint?}"
		else
			panic "Unable to find a live file system on the network"
		fi
	else
		if [ -n "${ISCSI_PORTAL}" ]
		then
			do_iscsi && livefs_root="${mountpoint}"
		elif [ -n "${PLAIN_ROOT}" ] && [ -n "${ROOT}" ]
		then
			# Do a local boot from hd
			livefs_root=${ROOT}
		else
			if [ -x /usr/bin/memdiskfind ]
			then
				if MEMDISK=$(/usr/bin/memdiskfind)
				then
					# We found a memdisk, set up phram
					# Sometimes "modprobe phram" can not successfully create /dev/mtd0.
				        # Have to try several times.
					max_try=20
					while [ ! -c /dev/mtd0 ] && [ "$max_try" -gt 0 ]; do
					  modprobe phram "phram=memdisk,${MEMDISK}"
					  sleep 0.2
					  if [ -c /dev/mtd0 ]; then
					  	break
					  else
					  	rmmod phram
				  	  fi
					  max_try=$((max_try - 1))
					done

					# Load mtdblock, the memdisk will be /dev/mtdblock0
					modprobe mtdblock
				fi
			fi

			# Scan local devices for the image
			i=0
			while [ "$i" -lt 60 ]
			do
				livefs_root=$(find_livefs ${i})

				if [ -n "${livefs_root}" ]
				then
					break
				fi

				sleep 1
				i=$((i + 1))
			done
		fi
	fi

	if [ -z "${livefs_root}" ]
	then
		panic "Unable to find a medium containing a live file system"
	fi

	Verify_checksums "${livefs_root}"

	if [ "${TORAM}" ]
	then
		live_dest="ram"
	elif [ "${TODISK}" ]
	then
		live_dest="${TODISK}"
	fi

	if [ "${live_dest}" ]
	then
		log_begin_msg "Copying live media to ${live_dest}"
		copy_live_to "${livefs_root}" "${live_dest}"
		log_end_msg
	fi

	# if we do not unmount the ISO we can't run "fsck /dev/ice" later on
	# because the mountpoint is left behind in /proc/mounts, so let's get
	# rid of it when running from RAM
	if [ -n "$FROMISO" ] && [ "${TORAM}" ]
	then
		losetup -d /dev/loop0

		if is_mountpoint /run/live/fromiso
		then
			umount /run/live/fromiso
			rmdir --ignore-fail-on-non-empty /run/live/fromiso \
				>/dev/null 2>&1 || true
		fi
	fi

	if [ -n "${MODULETORAMFILE}" ] || [ -n "${PLAIN_ROOT}" ]
	then
		setup_unionfs "${livefs_root}" "${rootmnt?}"
	else
		mac="$(get_mac)"
		mac="$(echo "${mac}" | sed 's/-//g')"
		mount_images_in_directory "${livefs_root}" "${rootmnt}" "${mac}"
	fi

	if [ -n "${ROOT_PID}" ]
	then
		echo "${ROOT_PID}" > "${rootmnt}"/lib/live/root.pid
	fi

	log_end_msg

	# aufs2 in kernel versions around 2.6.33 has a regression:
	# directories can't be accessed when read for the first the time,
	# causing a failure for example when accessing /var/lib/fai
	# when booting FAI, this simple workaround solves it
	ls /root/* >/dev/null 2>&1

	# if we do not unmount the ISO we can't run "fsck /dev/ice" later on
	# because the mountpoint is left behind in /proc/mounts, so let's get
	# rid of it when running from RAM
	if [ -n "$FINDISO" ] && [ "${TORAM}" ]
	then
		losetup -d /dev/loop0

		if is_mountpoint /run/live/findiso
		then
			umount /run/live/findiso
			rmdir --ignore-fail-on-non-empty /run/live/findiso \
				>/dev/null 2>&1 || true
		fi
	fi

	if [ -f /etc/hostname ] && ! grep -E -q -v '^[[:space:]]*(#|$)' "${rootmnt}/etc/hostname"
	then
		log_begin_msg "Copying /etc/hostname to ${rootmnt}/etc/hostname"
		cp -v /etc/hostname "${rootmnt}/etc/hostname"
		log_end_msg
	fi

	if [ -f /etc/hosts ] && ! grep -E -q -v '^[[:space:]]*(#|$|(127.0.0.1|::1|ff02::[12])[[:space:]])' "${rootmnt}/etc/hosts"
	then
		log_begin_msg "Copying /etc/hosts to ${rootmnt}/etc/hosts"
		cp -v /etc/hosts "${rootmnt}/etc/hosts"
		log_end_msg
	fi

	if [ -L /root/etc/resolv.conf ] ; then
		# assume we have resolvconf
		DNSFILE="${rootmnt}/etc/resolvconf/resolv.conf.d/base"
	else
		DNSFILE="${rootmnt}/etc/resolv.conf"
	fi
	if [ -f /etc/resolv.conf ] && ! grep -E -q -v '^[[:space:]]*(#|$)' "${DNSFILE}"
	then
		log_begin_msg "Copying /etc/resolv.conf to ${DNSFILE}"
		cp -v /etc/resolv.conf "${DNSFILE}"
		log_end_msg
	fi

	if ! [ -d "/lib/live/boot" ]
	then
		panic "A wrong rootfs was mounted."
	fi

	# avoid breaking existing user scripts that rely on the old path
	# this includes code that checks what is mounted on /lib/live/mount/*
	# (eg: grep /lib/live /proc/mount)
	# XXX: to be removed before the bullseye release
	mkdir -p "${rootmnt}/lib/live/mount"
	mount --rbind /run/live "${rootmnt}/lib/live/mount"

	Fstab
	Netbase

	Swap

	exec 1>&6 6>&-
	exec 2>&7 7>&-
	kill ${tailpid}
	[ -w "${rootmnt}/var/log/" ] && mkdir -p "${rootmnt}/var/log/live" && ( \
				cp boot.log "${rootmnt}/var/log/live" 2>/dev/null; \
				cp fsck.log "${rootmnt}/var/log/live" 2>/dev/null )
}
