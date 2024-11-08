arch_get_kernel_flavour () {
	case "$SUBARCH" in
	    kirkwood|orion5x)
		echo "marvell"
		return 0 ;;
	    versatile)
		echo "$SUBARCH"
		return 0 ;;
	    *)
		warning "Unknown $ARCH subarchitecture '$SUBARCH'."
		return 1 ;;
	esac
}

arch_check_usable_kernel () {
	case "$1" in
	    *-dbg)
		return 1
		;;
	    *-$2|*-$2-*)
		return 0
		;;
	    *)
		return 1
		;;
	esac
}

arch_get_kernel () {
	echo "linux-image-$1"
}
