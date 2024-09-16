arch_get_kernel_flavour () {
	echo "$MACHINE"
	return 0
}

arch_check_usable_kernel () {
	if echo "$1" | grep -Eq -- "-sparc64(-.*)?$"; then return 0; fi
	return 1
}

arch_get_kernel () {
	imgbase=linux-image

	CPUS=`grep 'ncpus probed' "$CPUINFO" | cut -d: -f2`
	TYPE=`grep '^type' "$CPUINFO" | head -n1 | cut -d: -f2 | sed -e 's/^[[:space:]]//'`
	if [ "$CPUS" -ne 1 ] || [ "$TYPE" = "sun4v" ]; then
		echo "$imgbase-$1-smp"
	fi
	echo "$imgbase-$1"
}
