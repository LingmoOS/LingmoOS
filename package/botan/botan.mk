################################################################################
#
# botan
#
################################################################################

BOTAN_VERSION = 3.3.0
BOTAN_SOURCE = Botan-$(BOTAN_VERSION).tar.xz
BOTAN_SITE = http://botan.randombit.net/releases
BOTAN_LICENSE = BSD-2-Clause
BOTAN_LICENSE_FILES = license.txt
BOTAN_CPE_ID_VALID = YES

BOTAN_INSTALL_STAGING = YES

BOTAN_DEPENDENCIES = host-python3
BOTAN_CONF_OPTS = \
	--cpu=$(LINGMO_ARCH) \
	--disable-cc-tests \
	--os=linux \
	--cc=gcc \
	--cc-bin="$(TARGET_CXX)" \
	--prefix=/usr \
	--without-documentation

ifeq ($(LINGMO_TOOLCHAIN_HAS_LIBATOMIC),y)
BOTAN_CONF_OPTS += --extra-libs=atomic
endif

ifeq ($(LINGMO_SHARED_LIBS),y)
BOTAN_CONF_OPTS += \
	--disable-static-library \
	--enable-shared-library
else ifeq ($(LINGMO_STATIC_LIBS),y)
BOTAN_CONF_OPTS += \
	--disable-shared-library \
	--enable-static-library \
	--no-autoload
else ifeq ($(LINGMO_SHARED_STATIC_LIBS),y)
BOTAN_CONF_OPTS += \
	--enable-shared-library \
	--enable-static-library
endif

ifeq ($(LINGMO_TOOLCHAIN_HAS_SSP),y)
BOTAN_CONF_OPTS += --with-stack-protector
else
BOTAN_CONF_OPTS += --without-stack-protector
endif

ifeq ($(LINGMO_TOOLCHAIN_HAS_THREADS_NPTL),y)
BOTAN_CONF_OPTS += --with-os-feature=threads
else
BOTAN_CONF_OPTS += --without-os-feature=threads
endif

ifeq ($(LINGMO_TOOLCHAIN_USES_UCLIBC),y)
BOTAN_CONF_OPTS += --without-os-feature=explicit_bzero,getauxval,getentropy
endif

ifeq ($(LINGMO_PACKAGE_BOOST_FILESYSTEM)$(LINGMO_PACKAGE_BOOST_SYSTEM),yy)
BOTAN_DEPENDENCIES += boost
BOTAN_CONF_OPTS += --with-boost
endif

ifeq ($(LINGMO_PACKAGE_BZIP2),y)
BOTAN_DEPENDENCIES += bzip2
BOTAN_CONF_OPTS += --with-bzip2
endif

ifeq ($(LINGMO_PACKAGE_SQLITE),y)
BOTAN_DEPENDENCIES += sqlite
BOTAN_CONF_OPTS += --with-sqlite
endif

ifeq ($(LINGMO_PACKAGE_TROUSERS),y)
BOTAN_DEPENDENCIES += trousers
BOTAN_CONF_OPTS += --with-tpm
endif

ifeq ($(LINGMO_PACKAGE_XZ),y)
BOTAN_DEPENDENCIES += xz
BOTAN_CONF_OPTS += --with-lzma
endif

ifeq ($(LINGMO_PACKAGE_ZLIB),y)
BOTAN_DEPENDENCIES += zlib
BOTAN_CONF_OPTS += --with-zlib
endif

ifeq ($(LINGMO_POWERPC_CPU_HAS_ALTIVEC),)
BOTAN_CONF_OPTS += --disable-altivec
endif

ifeq ($(LINGMO_ARM_CPU_HAS_NEON),)
BOTAN_CONF_OPTS += --disable-neon
endif

ifeq ($(LINGMO_SOFT_FLOAT),y)
BOTAN_CONF_OPTS += --disable-neon
endif

define BOTAN_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_MAKE_ENV) ./configure.py $(BOTAN_CONF_OPTS))
endef

define BOTAN_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) AR="$(TARGET_AR)"
endef

define BOTAN_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR="$(STAGING_DIR)" install
endef

define BOTAN_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D) DESTDIR="$(TARGET_DIR)" install
endef

$(eval $(generic-package))
