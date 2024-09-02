arch_get_kernel_flavour () {
	echo amd64
}

arch_check_usable_kernel () {
	case "$1" in
	    *-dbg)
		return 1
		;;
	    *-amd64 | *-amd64-*)
		# Allow any other hyphenated suffix
		return 0
		;;
	    *)
		return 1
		;;
	esac
}

arch_get_kernel () {
	echo "linux-image-amd64"
}
