# configuration options for all flavours
extra_config_options = --enable-multi-arch

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 64-bit (ppc64) alternative library
GLIBC_PASSES += ppc64
DEB_ARCH_MULTILIB_PACKAGES += libc6-ppc64 libc6-dev-ppc64
libc6-ppc64_shlib_dep = libc6-ppc64 (>= $(shlib_dep_ver))
ppc64_configure_target = powerpc64-linux-gnu
ppc64_CC = $(CC) -m64
ppc64_CXX = $(CXX) -m64
ppc64_rtlddir = /lib64
ppc64_slibdir = /lib64
ppc64_libdir = /usr/lib64

define libc6-dev-ppc64_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-ppc64)

mkdir -p debian/libc6-dev-ppc64/usr/include/powerpc-linux-gnu/gnu
cp -a debian/tmp-ppc64/usr/include/gnu/lib-names-64-v1.h \
	debian/tmp-ppc64/usr/include/gnu/stubs-64-v1.h \
	debian/libc6-dev-ppc64/usr/include/powerpc-linux-gnu/gnu

endef

endif # multilib
