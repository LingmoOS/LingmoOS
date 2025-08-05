# main library
libc_rtlddir = /lib64

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

# build 32-bit (o32) alternative library
GLIBC_PASSES += mips32
DEB_ARCH_MULTILIB_PACKAGES += libc6-mips32 libc6-dev-mips32
libc6-mips32_shlib_dep = libc6-mips32 (>= $(shlib_dep_ver))
mips32_configure_target = mipsisa32r6-linux-gnu
mips32_CC = $(CC) -mabi=32
mips32_CXX = $(CXX) -mabi=32
mips32_rtlddir = /lib
mips32_slibdir = /libo32
mips32_libdir = /usr/libo32

define libc6-dev-mipsn32_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-mipsn32)

mkdir -p debian/libc6-dev-mipsn32/usr/include/mipsisa64r6-linux-gnuabi64/gnu
cp -a debian/tmp-mipsn32/usr/include/gnu/lib-names-n32_hard_2008.h \
	debian/tmp-mipsn32/usr/include/gnu/stubs-n32_hard_2008.h \
	debian/libc6-dev-mipsn32/usr/include/mipsisa64r6-linux-gnuabi64/gnu

endef

define libc6-dev-mips32_extra_pkg_install

mkdir -p debian/libc6-dev-mips32/usr/include/mipsisa64r6-linux-gnuabi64/gnu
cp -a debian/tmp-mips32/usr/include/gnu/lib-names-o32_hard_2008.h \
	debian/tmp-mips32/usr/include/gnu/stubs-o32_hard_2008.h \
	debian/libc6-dev-mips32/usr/include/mipsisa64r6-linux-gnuabi64/gnu

endef

endif # multilib
