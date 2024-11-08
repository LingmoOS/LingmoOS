arch_get_kernel_flavour () {
	case "$SUBARCH" in
	    rts7751r2d)
		echo "sh7751r"
		return 0 ;;
	    sh7785lcr)
		echo "sh7785lcr"
		return 0 ;;
	    *)
		warning "Unknown $ARCH subarchitecture '$SUBARCH'."
		return 1 ;;
	esac
}

arch_check_usable_kernel () {
	# Subarchitecture must match exactly
	if echo "$1" | grep -Eq -- "-$2(-.*)?$"; then return 0; fi
	return 1
}

arch_get_kernel () {
	echo "linux-image-$1"
}
