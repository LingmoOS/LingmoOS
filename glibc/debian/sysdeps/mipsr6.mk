# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 32-bit (n32) alternative library
GLIBC_PASSES += mipsn32
DEB_ARCH_MULTILIB_PACKAGES += libc6-mipsn32 libc6-dev-mipsn32
libc6-mipsn32_shlib_dep = libc6-mipsn32 (>= $(shlib_dep_ver))
mipsn32_configure_target = mipsisa64r6-linux-gnuabin32
mipsn32_CC = $(CC) -mabi=n32
mipsn32_CXX = $(CXX) -mabi=n32
mipsn32_rtlddir = /lib32
mipsn32_slibdir = /lib32
mipsn32_libdir = /usr/lib32

# build 64-bit alternative library
GLIBC_PASSES += mips64
DEB_ARCH_MULTILIB_PACKAGES += libc6-mips64 libc6-dev-mips64
libc6-mips64_shlib_dep = libc6-mips64 (>= $(shlib_dep_ver))
mips64_configure_target = mipsisa64r6-linux-gnuabi64
mips64_CC = $(CC) -mabi=64
mips64_CXX = $(CXX) -mabi=64
mips64_rtlddir = /lib64
mips64_slibdir = /lib64
mips64_libdir = /usr/lib64

define libc6-dev-mips64_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-mips64)

mkdir -p debian/libc6-dev-mips64/usr/include/mipsisa32r6-linux-gnu/gnu
cp -a debian/tmp-mips64/usr/include/gnu/lib-names-n64_hard_2008.h \
	debian/tmp-mips64/usr/include/gnu/stubs-n64_hard_2008.h \
	debian/libc6-dev-mips64/usr/include/mipsisa32r6-linux-gnu/gnu

endef

define libc6-dev-mipsn32_extra_pkg_install

mkdir -p debian/libc6-dev-mipsn32/usr/include/mipsisa32r6-linux-gnu/gnu
cp -a debian/tmp-mipsn32/usr/include/gnu/lib-names-n32_hard_2008.h \
	debian/tmp-mipsn32/usr/include/gnu/stubs-n32_hard_2008.h \
	debian/libc6-dev-mipsn32/usr/include/mipsisa32r6-linux-gnu/gnu

endef

# Need to put a tri-arch aware version of ldd in the base package
define mipsn32_extra_install
cp debian/tmp-mipsn32/usr/bin/ldd debian/tmp/usr/bin
endef

endif # multilib
