# configuration options for all flavours
CC = $(DEB_HOST_GNU_TYPE)-$(BASE_CC)$(DEB_GCC_VERSION) -no-pie -fno-PIE
CXX = $(DEB_HOST_GNU_TYPE)-$(BASE_CXX)$(DEB_GCC_VERSION) -no-pie -fno-PIE
extra_config_options = --disable-multi-arch --disable-default-pie

# main library
libc_rtlddir = /lib64

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 32-bit (sparc) alternative library
GLIBC_PASSES += sparc
DEB_ARCH_MULTILIB_PACKAGES += libc6-sparc libc6-dev-sparc
libc6-sparc_shlib_dep = libc6-sparc (>= $(shlib_dep_ver))
sparc_configure_target=sparcv9-linux-gnu
sparc_CC = $(CC) -m32
sparc_CXX = $(CXX) -m32
sparc_rtlddir = /lib
sparc_slibdir = /lib32
sparc_libdir = /usr/lib32

define libc6-dev-sparc_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-sparc)

mkdir -p debian/libc6-dev-sparc/usr/include/sparc64-linux-gnu/gnu
cp -a debian/tmp-sparc/usr/include/gnu/lib-names-32.h \
	debian/tmp-sparc/usr/include/gnu/stubs-32.h \
	debian/libc6-dev-sparc/usr/include/sparc64-linux-gnu/gnu

endef

endif # multilib
