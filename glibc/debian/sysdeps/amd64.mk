# configuration options for all flavours
extra_config_options = --enable-multi-arch
CC = $(DEB_HOST_GNU_TYPE)-$(BASE_CC)$(DEB_GCC_VERSION) -Wl,--hash-style=both
CXX = $(DEB_HOST_GNU_TYPE)-$(BASE_CXX)$(DEB_GCC_VERSION) -Wl,--hash-style=both

# main library
libc_mvec = yes
libc_rtlddir = /lib64

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 32-bit (i386) alternative library
GLIBC_PASSES += i386
DEB_ARCH_MULTILIB_PACKAGES += libc6-i386 libc6-dev-i386
libc6-i386_shlib_dep = libc6-i386 (>= $(shlib_dep_ver))
i386_configure_target = i686-linux-gnu
i386_CC = $(CC) -m32
i386_CXX = $(CXX) -m32
i386_slibdir = /lib32
i386_libdir = /usr/lib32

define libc6-dev-i386_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-i386)

mkdir -p debian/libc6-dev-i386/usr/include/x86_64-linux-gnu/gnu
cp -a debian/tmp-i386/usr/include/gnu/lib-names-32.h \
	debian/tmp-i386/usr/include/gnu/stubs-32.h \
	debian/libc6-dev-i386/usr/include/x86_64-linux-gnu/gnu

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

mkdir -p debian/libc6-dev-x32/usr/include/x86_64-linux-gnu/gnu
cp -a debian/tmp-x32/usr/include/gnu/lib-names-x32.h \
	debian/tmp-x32/usr/include/gnu/stubs-x32.h \
	debian/libc6-dev-x32/usr/include/x86_64-linux-gnu/gnu/

endef

endif # multilib
