# main library
libc_rtlddir = /lib32

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 64-bit alternative library
GLIBC_PASSES += mips64
DEB_ARCH_MULTILIB_PACKAGES += libc6-mips64 libc6-dev-mips64
libc6-mips64_shlib_dep = libc6-mips64 (>= $(shlib_dep_ver))
mips64_configure_target = mipsisa64r6el-linux-gnuabi64
mips64_CC = $(CC) -mabi=64
mips64_CXX = $(CXX) -mabi=64
mips64_rtlddir = /lib64
mips64_slibdir = /lib64
mips64_libdir = /usr/lib64

# build 32-bit (o32) alternative library
GLIBC_PASSES += mips32
DEB_ARCH_MULTILIB_PACKAGES += libc6-mips32 libc6-dev-mips32
libc6-mips32_shlib_dep = libc6-mips32 (>= $(shlib_dep_ver))
mips32_configure_target = mipsisa32r6el-linux-gnu
mips32_CC = $(CC) -mabi=32
mips32_CXX = $(CXX) -mabi=32
mips32_rtlddir = /lib
mips32_slibdir = /libo32
mips32_libdir = /usr/libo32

define libc6-dev-mips64_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-mips64)

mkdir -p debian/libc6-dev-mips64/usr/include/mipsisa64r6el-linux-gnuabin32/gnu
cp -a debian/tmp-mips64/usr/include/gnu/lib-names-n64_hard_2008.h \
	debian/tmp-mips64/usr/include/gnu/stubs-n64_hard_2008.h \
	debian/libc6-dev-mips64/usr/include/mipsisa64r6el-linux-gnuabin32/gnu

endef

define libc6-dev-mips32_extra_pkg_install

mkdir -p debian/libc6-dev-mips32/usr/include/mipsisa64r6el-linux-gnuabin32/gnu
cp -a debian/tmp-mips32/usr/include/gnu/lib-names-o32_hard_2008.h \
	debian/tmp-mips32/usr/include/gnu/stubs-o32_hard_2008.h \
	debian/libc6-dev-mips32/usr/include/mipsisa64r6el-linux-gnuabin32/gnu

endef

endif # multilib
