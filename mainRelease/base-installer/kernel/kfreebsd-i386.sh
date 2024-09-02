arch_get_kernel_flavour () {
	VENDOR=`grep '^vendor_id' "$CPUINFO" | head -n1 | cut -d: -f2`
	FAMILY=`grep '^cpu family' "$CPUINFO" | head -n1 | cut -d: -f2`
	MODEL=`grep '^model[[:space:]]*:' "$CPUINFO" | head -n1 | cut -d: -f2`
	NUMCPUS=`grep ^processor "$CPUINFO" | wc -l`

	# Only offer smp if the system supports has more than one cpu
	if test $NUMCPUS -gt "1" ; then
	    SMP="-smp"
	fi

	case "$VENDOR" in
	    " AuthenticAMD"*)
		case "$FAMILY" in
		    " 15"|" 16"|" 17"|" 18"|" 20")	# k8
			echo 686$SMP
			;;
		    " 6")				# k7
			case "$MODEL" in
			    " 0"|" 1"|" 2"|" 3"|" 4"|" 5")
				# May not have SSE support
				echo 486 ;;
			    *)	echo 686$SMP ;;
			esac
			;;
		    *)		echo 486 ;;
		esac
		;;
	    " GenuineIntel")
		case "$FAMILY" in
		    " 6"|" 15")	echo 686$SMP ;;
		    *)		echo 486 ;;
		esac
		;;
	    " CentaurHauls")
		case "$FAMILY" in
		    " 6")
			case "$MODEL" in
			    " 9"|" 10")	echo 686$SMP ;;
			    *)		echo 486 ;;
			esac
			;;
		    *)
			echo 486 ;;
		esac
		;;
	    *)
		echo 486 ;;
	esac
	return 0
}

arch_check_usable_kernel () {
	if echo "$1" | grep -Eq -- "-486(-.*)?$"; then return 0; fi
	if [ "$2" = 486 ]; then return 1; fi
	if echo "$1" | grep -Eq -- "-686(-.*)?$"; then return 0; fi
	if [ "$2" = 686 ] || [ "$2" = 686-smp ]; then return 1; fi

	# default to usable in case of strangeness
	warning "Unknown kernel usability: $1 / $2"
	return 0
}

arch_get_kernel () {
	if [ "$1" = 686-smp ]; then
		echo "kfreebsd-image-$KERNEL_MAJOR-686-smp"
	fi
	if [ "$1" = 686 ]; then
		echo "kfreebsd-image-$KERNEL_MAJOR-686"
	fi
	echo "kfreebsd-image-$KERNEL_MAJOR-486"
}
