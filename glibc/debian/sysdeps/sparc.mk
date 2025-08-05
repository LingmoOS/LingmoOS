# configuration options for all flavours
extra_config_options = --disable-multi-arch

# main library
libc_configure_target=sparcv9-linux-gnu

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 64-bit (sparc64) alternative library
GLIBC_PASSES += sparc64
DEB_ARCH_MULTILIB_PACKAGES += libc6-sparc64 libc6-dev-sparc64
libc6-sparc64_shlib_dep = libc6-sparc64 (>= $(shlib_dep_ver))
sparc64_configure_target=sparc64-linux-gnu
sparc64_CC = $(CC) -m64
sparc64_CXX = $(CXX) -m64
sparc64_rtlddir = /lib64
sparc64_slibdir = /lib64
sparc64_libdir = /usr/lib64

define libc6-dev-sparc64_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-sparc64)

mkdir -p debian/libc6-dev-sparc64/usr/include/sparc-linux-gnu/gnu
cp -a debian/tmp-sparc64/usr/include/gnu/lib-names-64.h \
	debian/tmp-sparc64/usr/include/gnu/stubs-64.h \
	debian/libc6-dev-sparc64/usr/include/sparc-linux-gnu/gnu

endef

endif # multilib
