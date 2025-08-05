# configuration options for all flavours
extra_config_options = --enable-multi-arch

# multilib flavours
ifeq (,$(filter nobiarch, $(DEB_BUILD_PROFILES)))

# build 32-bit (s390) alternative library
GLIBC_PASSES += s390
DEB_ARCH_MULTILIB_PACKAGES += libc6-s390 libc6-dev-s390
libc6-s390_shlib_dep = libc6-s390 (>= $(shlib_dep_ver))
s390_configure_target = s390-linux-gnu
s390_CC = $(CC) -m31
s390_CXX = $(CXX) -m31
s390_slibdir = /lib32
s390_libdir = /usr/lib32

define libc6-dev-s390_extra_pkg_install

$(call generic_multilib_extra_pkg_install,libc6-dev-s390)

mkdir -p debian/libc6-dev-s390/usr/include/s390x-linux-gnu/gnu
cp -a debian/tmp-s390/usr/include/gnu/lib-names-32.h \
	debian/tmp-s390/usr/include/gnu/stubs-32.h \
	debian/libc6-dev-s390/usr/include/s390x-linux-gnu/gnu

endef

endif # multilib
