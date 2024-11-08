# kernel/riscv64.sh
# Copyright 2019 Karsten Merker <merker@debian.org>
# License: GPLv2

# The riscv64 architecture has only a single unified multiplatform kernel
# (which uses "riscv64" as its flavour name). Therefore we don't need to
# differentiate between multiple flavours here.

arch_get_kernel_flavour () {
	echo riscv64
}

arch_check_usable_kernel () {
	# parameters: $1=kernel, $2=flavour
	case "$1" in
	    *-dbg)
		# Don't show debug packages in the kernel selection list.
		return 1
		;;
	    *-riscv64 | *-riscv64-*)
		return 0
		;;
	    *)
		return 1
		;;
	esac
}

arch_get_kernel () {
	# parameters: $1=flavour
	echo "linux-image-riscv64"
}
