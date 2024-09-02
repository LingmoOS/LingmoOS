arch_get_kernel_flavour () {
	echo $MACHINE
	return 0
}

arch_check_usable_kernel () {
	case "$1" in
	    *-s390x-tape)
		# Bastian Blank says: "-s390-tape is only a kernel
		# image without any logic and modules"
		return 1 ;;
	    *)
		return 0 ;;
	esac
}

arch_get_kernel () {
	echo "linux-image-$1"
}
