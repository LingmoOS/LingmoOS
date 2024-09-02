#!/bin/sh

Wait_for_carrier ()
{
	# $1 = network device
	echo -n "Waiting for link to come up on $1... "
	ip link set $1 up
	for step in $(seq 1 15)
	do
		carrier=$(cat /sys/class/net/$1/carrier \
		2>/dev/null)
		case "${carrier}" in
			1)
			echo -e "\nLink is up"
			return
			;;
			*)
			# Counter
			echo -n "$step "
			;;
		esac
		sleep 1
	done
	echo -e "\nError - carrier not detected on $1."
	ip link set $1 down
}

Select_eth_device ()
{
	# Boot type in initramfs's config
	bootconf=$(egrep '^BOOT=' /conf/initramfs.conf | tail -1)

	# can be superseded by command line (used by Debian-Live's netboot for example)
	for ARGUMENT in ${LIVE_BOOT_CMDLINE}
	do
		case "${ARGUMENT}" in
			netboot=*)
				NETBOOT="${ARGUMENT#netboot=}"
				;;
		esac
	done

	if [ "$bootconf" != "BOOT=nfs" ] && [ -z "$NETBOOT" ] && [ -z "$FETCH" ] && [ -z "$FTPFS" ] && [ -z "$HTTPFS" ]
	then
		# Not a net boot : nothing to do
		return
	fi

	# we want to do some basic IP
	modprobe -q af_packet

	# Ensure all our net modules get loaded so we can actually compare MAC addresses...
	udevadm trigger
	udevadm settle

	# Available Ethernet interfaces ?
	l_interfaces=""

	# See if we can derive the boot device
	Device_from_bootif

	if [ -z "$DEVICE" ]
	then
		echo "Waiting for ethernet card(s) up... If this fails, maybe the ethernet card is not supported by the kernel `uname -r`?"
		while [ -z "$l_interfaces" ]
		do
			l_interfaces="$(cd /sys/class/net/ && ls -d * 2>/dev/null | grep -v "lo")"
		done

		if [ $(echo $l_interfaces | wc -w) -lt 2 ]
		then
			# only one interface : no choice
			echo "DEVICE=$l_interfaces" >> /conf/param.conf
			Wait_for_carrier $l_interfaces
			return
		fi

		# If user force to use specific device, write it
		for ARGUMENT in ${LIVE_BOOT_CMDLINE}
		do
			case "${ARGUMENT}" in
				live-netdev=*)
				NETDEV="${ARGUMENT#live-netdev=}"
				echo "DEVICE=$NETDEV" >> /conf/param.conf
				echo "Found live-netdev parameter, forcing to to use network device $NETDEV."
				Wait_for_carrier $NETDEV
				return
				;;
			esac
		done
	else
		l_interfaces="$DEVICE"
	fi

	found_eth_dev=""
	while true
	do
		echo -n "Looking for a connected Ethernet interface ..."

		for interface in $l_interfaces
		do
			# ATTR{carrier} is not set if this is not done
			echo -n " $interface ?"
			ipconfig -c none -d $interface -t 1 >/dev/null 2>&1
			sleep 1
		done

		echo ''

		for step in 1 2 3 4 5
		do
			for interface in $l_interfaces
			do
				ip link set $interface up
				carrier=$(cat /sys/class/net/$interface/carrier \
					2>/dev/null)
				# link detected

				case "${carrier}" in
					1)
						echo "Connected $interface found"
						# inform initrd's init script :
						found_eth_dev="$found_eth_dev $interface"
						found_eth_dev="$(echo $found_eth_dev | sed -e "s/^[[:space:]]*//g")"
						;;
				esac
			done
			if [ -n "$found_eth_dev" ]
			then
				echo "DEVICE='$found_eth_dev'" >> /conf/param.conf
				return
			else
				# wait a bit
				sleep 1
			fi
		done
	done
}
