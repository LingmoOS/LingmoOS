libc_packages := libc6 libc6.1 libc0.3
libc0_3_archs := hurd-i386 hurd-amd64
libc6_archs   := amd64 arc arm64 armel armhf hppa i386 loong64 m68k \
                 mips mipsel mipsn32 mipsn32el mips64 mips64el mipsr6 mipsr6el \
                 mipsn32r6 mipsn32r6el mips64r6 mips64r6el nios2 powerpc ppc64 ppc64el riscv64 \
                 sparc sparc64 s390x sh3 sh4 x32
libc6_1_archs := alpha

control_deps := $(wildcard debian/control.in/*) $(addprefix debian/control.in/, $(libc_packages))
triggers := binutils, linux-libc-dev [linux-any], $(BASE_CC)$(DEB_GCC_VERSION)

$(patsubst %,debian/control.in/%,$(libc_packages)) :: debian/control.in/% : debian/control.in/libc debian/rules.d/control.mk
	sed -e "s%@libc@%$*%g" \
	    -e "s%@archs@%$($(subst .,_,$*)_archs)%g" \
	    -e "s%@libc-dev-conflict@%$(foreach arch,$(filter-out $*,$(libc_packages)),$(arch)-dev,)%g" \
	    < $< > $@

debian/control: $(stamp)control
$(stamp)control: debian/rules.d/control.mk $(control_deps) debian/tests/control.in

	# Check that all files end with a new line
	set -e ; for i in debian/control.in/* ; do \
		tail -n1 $$i | grep -q "^$$" ; \
	done

	cat debian/control.in/main		>  $@T
	cat debian/control.in/libc6		>> $@T
	cat debian/control.in/libc6.1		>> $@T
	cat debian/control.in/libc0.3		>> $@T
	cat debian/control.in/i386		>> $@T
	cat debian/control.in/sparc		>> $@T
	cat debian/control.in/sparc64		>> $@T
	cat debian/control.in/s390 		>> $@T
	cat debian/control.in/amd64		>> $@T
	cat debian/control.in/powerpc		>> $@T
	cat debian/control.in/ppc64		>> $@T
	cat debian/control.in/mips32		>> $@T
	cat debian/control.in/mipsn32		>> $@T
	cat debian/control.in/mips64		>> $@T
	cat debian/control.in/x32		>> $@T
	sed -e 's%@libc@%$(libc)%g' -e 's%@DEB_VERSION_UPSTREAM@%$(DEB_VERSION_UPSTREAM)%g' -e 's%@DEB_GCC_VERSION@%$(DEB_GCC_VERSION)%g' < $@T > debian/control
	rm $@T

	# And generate the tests control file with the current GCC
	sed -e 's%@triggers@%$(triggers)%g' debian/tests/control.in > debian/tests/control
	touch $@
