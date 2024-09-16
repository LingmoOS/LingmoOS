#!/bin/sh

#set -e

do_iscsi()
{
	do_netsetup
	#modprobe ib_iser
	modprobe iscsi_tcp
	local debugopt
	debugopt=""
	[ "${LIVE_BOOT_DEBUG}" = "true" ] && debugopt="-d 8"
	#FIXME this name is supposed to be unique - some date + ifconfig hash?
	ISCSI_INITIATORNAME="iqn.1993-08.org.debian.live:01:$(echo "${HWADDR}" | sed -e s/://g)"
	export ISCSI_INITIATORNAME
	if [ -n "${ISCSI_SERVER}" ] ; then
		iscsistart $debugopt -i "${ISCSI_INITIATORNAME}" -t "${ISCSI_TARGET}" -g 1 -a "${ISCSI_SERVER}" -p "${ISCSI_PORT}"
	else
		iscsistart $debugopt -i "${ISCSI_INITIATORNAME}" -t "${ISCSI_TARGET}" -g 1 -a "${ISCSI_PORTAL}" -p 3260
	fi
	if [ $? != 0 ]
	then
		panic "Failed to log into iscsi target"
	fi
	local host
	host="$(ls -d /sys/class/scsi_host/host*/device/iscsi_host:host* \
		      /sys/class/scsi_host/host*/device/iscsi_host/host* | sed -e 's:/device.*::' -e 's:.*host::')"
	if [ -n "${host}" ]
	then
		local devices i
		devices=""
		i=0
		while [ -z "${devices}" -a $i -lt 60 ]
		do
			sleep 1
			devices="$(ls -d /sys/class/scsi_device/${host}*/device/block:* \
					 /sys/class/scsi_device/${host}*/device/block/* | sed -e 's!.*[:/]!!')"
			i=$(expr $i + 1)
			echo -ne $i\\r
		done
		for dev in $devices
		do
			if check_dev "null" "/dev/$dev"
			then
				NETBOOT="iscsi"
				export NETBOOT
				return 0;
			fi
		done
		panic "Failed to locate a live device on iSCSI devices (tried: $devices)."
	else
		panic "Failed to locate iSCSI host in /sys"
	fi
}
