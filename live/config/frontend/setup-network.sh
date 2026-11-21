#!/bin/sh

Setup_network ()
{
	if [ -e /var/lib/live/config/setup-network ]; then
		return
	fi
	if [ -e /etc/init.d/live-config ] || [ -e /lib/systemd/system/live-config.service ]
	then
		/etc/init.d/mountkernfs.sh start > /dev/null 2>&1
		/etc/init.d/mountdevsubfs.sh start > /dev/null 2>&1
		/etc/init.d/networking start > /dev/null 2>&1

		# Now force adapter up if specified with either BOOTIF= or ethdevice= on cmdline
		for _PARAMETER in ${LIVE_CONFIG_CMDLINE}
		do
			case "${_PARAMETER}" in
				BOOTIF=*)
					BOOTIF="${_PARAMETER#*BOOTIF=}"
					;;
				ethdevice=*)
					ETHDEVICE="${_PARAMETER#*ethdevice=}"
					;;
			esac
		done
		if [ -n "${BOOTIF}" ]
		then
			# pxelinux sets BOOTIF to a value based on the mac address of the
			# network card used to PXE boot, so use this value for DEVICE rather
			# than a hard-coded device name from initramfs.conf. this facilitates
			# network booting when machines may have multiple network cards.
			# pxelinux sets BOOTIF to 01-$mac_address

			# strip off the leading "01-", which isn't part of the mac
			# address
			temp_mac=${BOOTIF#*-}

			# convert to typical mac address format by replacing "-" with ":"
			bootif_mac=""
			IFS='-'
			for x in $temp_mac
			do
				if [ -z "$bootif_mac" ]
				then
					bootif_mac="$x"
				else
					bootif_mac="$bootif_mac:$x"
				fi
			done
			unset IFS

			# look for devices with matching mac address, and set DEVICE to
			# appropriate value if match is found.

			for device in /sys/class/net/*
			do
				if [ -f "$device/address" ]
				then
					current_mac=$(cat "$device/address")

					if [ "$bootif_mac" = "$current_mac" ]
					then
						ifup --force "${device##*/}"
						break
					fi
				fi
			done
		elif [ -n "${ETHDEVICE}" ]
		then
			ifup --force "${ETHDEVICE}"
		fi
		touch /var/lib/live/config/setup-network
	fi
}
