# configuration options for all flavours
extra_config_options = --enable-multi-arch

# main library
libc_rtlddir = /lib64

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 32-bit (powerpc) alternative library
GLIBC_PASSES += powerpc
DEB_ARCH_MULTILIB_PACKAGES += libc6-powerpc libc6-dev-powerpc
libc6-powerpc_shlib_dep = libc6-powerpc (>= $(shlib_dep_ver))
powerpc_configure_target = powerpc-linux-gnu
powerpc_CC = $(CC) -m32
powerpc_CXX = $(CXX) -m32
powerpc_slibdir = /lib32
powerpc_libdir = /usr/lib32

define libc6-dev-powerpc_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-powerpc)

mkdir -p debian/libc6-dev-powerpc/usr/include/powerpc64-linux-gnu/gnu
cp -a debian/tmp-powerpc/usr/include/gnu/lib-names-32.h \
	debian/tmp-powerpc/usr/include/gnu/stubs-32.h \
	debian/libc6-dev-powerpc/usr/include/powerpc64-linux-gnu/gnu

endef

endif # multilib
