arch_get_kernel_flavour () {
	if grep '^features' "$CPUINFO" | grep -q branchlong; then
		echo mckinley
	else
		echo itanium
	fi
	return 0
}

arch_check_usable_kernel () {
	if echo "$1" | grep -Eq -- "-itanium(-.*)?$"; then return 0; fi
	if [ "$2" = itanium ]; then return 1; fi
	if echo "$1" | grep -Eq -- "-mckinley(-.*)?$"; then return 0; fi

	# default to usable in case of strangeness
	warning "Unknown kernel usability: $1 / $2"
	return 0
}

arch_get_kernel () {
	echo "linux-image-$1"
}
