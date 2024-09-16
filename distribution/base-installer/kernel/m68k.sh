arch_get_kernel_flavour () {
	echo m68k
}

arch_check_usable_kernel () {
	case "$1" in
	    *-dbg)
		return 1
		;;
	    *-m68k | *-m68k-*)
		# Allow any other hyphenated suffix
		return 0
		;;
	    *)
		return 1
		;;
	esac
}

arch_get_kernel () {
	echo "linux-image-m68k"
}
