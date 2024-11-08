arch_get_kernel_flavour () {
	case "$SUBARCH" in
	    4kc-malta|5kc-malta|octeon)
		echo "$SUBARCH"
		return 0 ;;
	    *)
		warning "Unknown $ARCH subarchitecture '$SUBARCH'."
		return 1 ;;
	esac
}

arch_check_usable_kernel () {
	# Subarchitecture must match exactly
	if echo "$1" | grep -Eq -- "-$2(-.*)?$"; then return 0; fi
	# The 4kc-malta kernel will do for 5kc-malta as well
	if [ "$2" = 5kc-malta ] && \
	   echo "$1" | grep -Eq -- "-4kc-malta(-.*)?$"; then
		return 0
	fi
	return 1
}

arch_get_kernel () {
	case $1 in
	    5kc-malta)
		echo "linux-image-$1"
		set 4kc-malta
		;;
	esac
	echo "linux-image-$1"
}
