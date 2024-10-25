arch_get_kernel_flavour () {
	VENDOR=`grep '^vendor_id' "$CPUINFO" | head -n1 | cut -d: -f2`
	case "$VENDOR" in
	    *)
		echo amd64 ;;
	esac
	return 0
}

arch_check_usable_kernel () {
	if echo "$1" | grep -Eq -- "-amd64(-.*)?$"; then return 0; fi

	return 1
}

arch_get_kernel () {
	echo "kfreebsd-image-$KERNEL_MAJOR-amd64"
}
