arch_get_kernel_flavour () {
	echo "arm64"
	return 0
}

arch_check_usable_kernel () {
	case "$1" in
	    *-dbg)
		return 1
		;;
	    *)
		return 0
		;;
	esac
}

arch_get_kernel () {
	echo "linux-image-arm64"
}
