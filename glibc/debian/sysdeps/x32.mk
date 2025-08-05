# configuration options for all flavours
extra_config_options = --enable-multi-arch
CC = $(DEB_HOST_GNU_TYPE)-$(BASE_CC)$(DEB_GCC_VERSION) -Wl,--hash-style=both
CXX = $(DEB_HOST_GNU_TYPE)-$(BASE_CXX)$(DEB_GCC_VERSION) -Wl,--hash-style=both

# main library
libc_mvec = yes
libc_rtlddir = /libx32

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 64-bit (amd64) alternative library
GLIBC_PASSES += amd64
DEB_ARCH_MULTILIB_PACKAGES += libc6-amd64 libc6-dev-amd64
libc6-amd64_shlib_dep = libc6-amd64 (>= $(shlib_dep_ver))
amd64_configure_target = x86_64-linux-gnu
amd64_CC = $(CC) -m64
amd64_CXX = $(CXX) -m64
amd64_mvec = yes
amd64_rtlddir = /lib64
amd64_slibdir = /lib64
amd64_libdir = /usr/lib64

define libc6-dev-amd64_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-amd64)

mkdir -p debian/libc6-dev-amd64/usr/include/x86_64-linux-gnux32/gnu
cp -a debian/tmp-amd64/usr/include/gnu/lib-names-64.h \
	debian/tmp-amd64/usr/include/gnu/stubs-64.h \
	debian/libc6-dev-amd64/usr/include/x86_64-linux-gnux32/gnu

endef

# build 32-bit (i386) alternative library
GLIBC_PASSES += i386
DEB_ARCH_MULTILIB_PACKAGES += libc6-i386 libc6-dev-i386
libc6-i386_shlib_dep = libc6-i386 (>= $(shlib_dep_ver))
i386_configure_target = i686-linux-gnu
i386_CC = $(CC) -m32
i386_CXX = $(CXX) -m32
i386_mvec = no
i386_slibdir = /lib32
i386_libdir = /usr/lib32

define libc6-dev-i386_extra_pkg_install

mkdir -p debian/libc6-dev-i386/usr/include/x86_64-linux-gnux32/gnu
cp -a debian/tmp-i386/usr/include/gnu/lib-names-32.h \
	debian/tmp-i386/usr/include/gnu/stubs-32.h \
	debian/libc6-dev-i386/usr/include/x86_64-linux-gnux32/gnu

endef

endif # multilib
