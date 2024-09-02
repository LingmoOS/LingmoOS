arch_get_kernel_flavour () {
	echo powerpc64le
	return 0
}

arch_check_usable_kernel () {
	return 0
}

arch_get_kernel () {
	echo "linux-image-powerpc64le"
}
