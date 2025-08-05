# configuration options for all flavours
extra_config_options = --enable-multi-arch
CC = $(DEB_HOST_GNU_TYPE)-$(BASE_CC)$(DEB_GCC_VERSION) -Wl,--hash-style=both
CXX = $(DEB_HOST_GNU_TYPE)-$(BASE_CXX)$(DEB_GCC_VERSION) -Wl,--hash-style=both

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 64-bit (amd64) alternative library
GLIBC_PASSES += amd64
DEB_ARCH_MULTILIB_PACKAGES += libc6-amd64 libc6-dev-amd64
libc6-amd64_shlib_dep = libc6-amd64 (>= $(shlib_dep_ver))
amd64_configure_target = x86_64-linux-gnu
# __x86_64__ is defined here because Makeconfig uses -undef and the
# /usr/include/asm wrappers need that symbol.
amd64_CC = $(CC) -m64 -D__x86_64__
amd64_CXX = $(CXX) -m64 -D__x86_64__
amd64_mvec = yes
amd64_rtlddir = /lib64
amd64_slibdir = /lib64
amd64_libdir = /usr/lib64

define amd64_extra_install
cp debian/tmp-amd64/usr/bin/ldd \
	debian/tmp/usr/bin
endef

define libc6-dev-amd64_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-amd64)

mkdir -p debian/libc6-dev-amd64/usr/include/i386-linux-gnu/gnu
cp -a debian/tmp-amd64/usr/include/gnu/lib-names-64.h \
	debian/tmp-amd64/usr/include/gnu/stubs-64.h \
	debian/libc6-dev-amd64/usr/include/i386-linux-gnu/gnu

endef

# build x32 ABI alternative library
GLIBC_PASSES += x32
DEB_ARCH_MULTILIB_PACKAGES += libc6-x32 libc6-dev-x32
libc6-x32_shlib_dep = libc6-x32 (>= $(shlib_dep_ver))
x32_configure_target = x86_64-linux-gnux32
x32_CC = $(CC) -mx32
x32_CXX = $(CXX) -mx32
x32_mvec = yes
x32_rtlddir = /libx32
x32_slibdir = /libx32
x32_libdir = /usr/libx32

define libc6-dev-x32_extra_pkg_install

mkdir -p debian/libc6-dev-x32/usr/include/i386-linux-gnu/gnu
cp -a debian/tmp-x32/usr/include/gnu/lib-names-x32.h \
	debian/tmp-x32/usr/include/gnu/stubs-x32.h \
	debian/libc6-dev-x32/usr/include/i386-linux-gnu/gnu

endef

endif # multilib
