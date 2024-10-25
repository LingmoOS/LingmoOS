arch_has_lpae () {
	if grep -q '^Features.*\blpae\b' "$CPUINFO"; then
		echo y
	else
		echo n
	fi
}

arch_get_kernel_flavour () {
	case "$SUBARCH" in
	    omap|mx5|vexpress)
		echo "$SUBARCH armmp"
		return 0 ;;
	    generic|efi)
		case `arch_has_lpae` in
		    y)
			echo "armmp-lpae armmp"
			;;
		    n)
			echo "armmp"
			;;
		esac
		return 0 ;;
	    *)
		warning "Unknown $ARCH subarchitecture '$SUBARCH'."
		return 1 ;;
	esac
}

arch_check_usable_kernel () {
        local NAME="$1"

        set -- $2
        while [ $# -ge 1 ]; do
                TRY="$1"
                case "$NAME" in
		    *-dbg)
			return 1
			;;
		    *-"$TRY"-lpae | *-"$TRY"-lpae-*)
                        # Allow any other hyphenated suffix
			if test `arch_has_lpae` = y ; then
				return 0
			fi
			;;
                    *-"$TRY" | *-"$TRY"-*)
                        # Allow any other hyphenated suffix
                        return 0
                        ;;
                esac
                shift
        done
        return 1
}

arch_get_kernel () {
	imgbase="linux-image"

	set -- $1
	while [ $# -ge 1 ]; do
		echo "$imgbase-$1"
		shift
	done
}
