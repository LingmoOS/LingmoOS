arch_get_kernel_flavour () {
	# TODO: test Xen, return xen-486 in that case
	echo 486
	return 0
}

arch_check_usable_kernel () {
	# TODO: test Xen
	return 0
}

arch_get_kernel () {
	echo "gnumach-image-$KERNEL_MAJOR-$1"
}
