################################################################################
#
# kvm-unit-tests
#
################################################################################

KVM_UNIT_TESTS_VERSION = 2024-01-08
KVM_UNIT_TESTS_SOURCE = kvm-unit-tests-v$(KVM_UNIT_TESTS_VERSION).tar.bz2
KVM_UNIT_TESTS_SITE = https://gitlab.com/kvm-unit-tests/kvm-unit-tests/-/archive/v$(KVM_UNIT_TESTS_VERSION)
KVM_UNIT_TESTS_LICENSE = GPL-2.0, LGPL-2.0
KVM_UNIT_TESTS_LICENSE_FILES = COPYRIGHT LICENSE

ifeq ($(LINGMO_aarch64)$(LINGMO_aarch64_be),y)
KVM_UNIT_TESTS_ARCH = aarch64
else ifeq ($(LINGMO_arm),y)
KVM_UNIT_TESTS_ARCH = arm
else ifeq ($(LINGMO_i386),y)
KVM_UNIT_TESTS_ARCH = i386
else ifeq ($(LINGMO_powerpc64)$(LINGMO_powerpc64le),y)
KVM_UNIT_TESTS_ARCH = ppc64
else ifeq ($(LINGMO_s390x),y)
KVM_UNIT_TESTS_ARCH = s390x
else ifeq ($(LINGMO_x86_64),y)
KVM_UNIT_TESTS_ARCH = x86_64
endif

ifeq ($(LINGMO_ENDIAN),"LITTLE")
KVM_UNIT_TESTS_ENDIAN = little
else
KVM_UNIT_TESTS_ENDIAN = big
endif

KVM_UNIT_TESTS_CONF_OPTS =\
	--disable-werror \
	--arch="$(KVM_UNIT_TESTS_ARCH)" \
	--processor="$(GCC_TARGET_CPU)" \
	--endian="$(KVM_UNIT_TESTS_ENDIAN)"

# For all architectures but x86-64, we use the target
# compiler. However, for x86-64, we use the host compiler, as
# kvm-unit-tests builds 32 bit code, which Buildroot toolchains for
# x86-64 cannot do.
ifeq ($(LINGMO_x86_64),)
KVM_UNIT_TESTS_CONF_OPTS += --cross-prefix="$(TARGET_CROSS)"
endif

define KVM_UNIT_TESTS_CONFIGURE_CMDS
	cd $(@D) && ./configure $(KVM_UNIT_TESTS_CONF_OPTS)
endef

define KVM_UNIT_TESTS_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(KVM_UNIT_TESTS_MAKE_OPTS) -C $(@D) \
		standalone
endef

define KVM_UNIT_TESTS_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(KVM_UNIT_TESTS_MAKE_OPTS) -C $(@D) \
		DESTDIR=$(TARGET_DIR)/usr/share/kvm-unit-tests/ \
		install
endef

# Does use configure script but not an autotools one
$(eval $(generic-package))
