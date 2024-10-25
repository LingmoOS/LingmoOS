################################################################################
#
# rhash
#
################################################################################

RHASH_VERSION = 1.4.4
RHASH_SOURCE = rhash-$(RHASH_VERSION)-src.tar.gz
RHASH_SITE = https://sourceforge.net/projects/rhash/files/rhash/$(RHASH_VERSION)
RHASH_LICENSE = 0BSD
RHASH_LICENSE_FILES = COPYING
RHASH_CPE_ID_VALID = YES
RHASH_INSTALL_STAGING = YES
RHASH_DEPENDENCIES = $(TARGET_NLS_DEPENDENCIES)

ifeq ($(LINGMO_SYSTEM_ENABLE_NLS),y)
RHASH_CONF_OPTS += --enable-gettext
else
RHASH_CONF_OPTS += --disable-gettext
endif

ifeq ($(LINGMO_PACKAGE_OPENSSL)x$(LINGMO_STATIC_LIBS),yx)
RHASH_CONF_OPTS += --enable-openssl
RHASH_DEPENDENCIES += openssl
else
RHASH_CONF_OPTS += --disable-openssl
endif

define RHASH_CONFIGURE_CMDS
	(cd $(@D); $(TARGET_CONFIGURE_OPTS) ./configure \
		--prefix=/usr \
		--cc=$(TARGET_CC) \
		--target=$(GNU_TARGET_NAME) \
		$(RHASH_CONF_OPTS) \
	)
endef

ifeq ($(LINGMO_SHARED_LIBS),y)
RHASH_BUILD_TARGETS = lib-shared build
RHASH_INSTALL_TARGETS = install-lib-shared install-so-link
else ifeq ($(LINGMO_STATIC_LIBS),y)
RHASH_BUILD_TARGETS = lib-static
RHASH_INSTALL_TARGETS = install-lib-static
else
RHASH_BUILD_TARGETS = lib-static lib-shared build
RHASH_INSTALL_TARGETS = install-lib-static install-lib-shared install-so-link
endif

define RHASH_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		$(RHASH_MAKE_OPTS) $(RHASH_BUILD_TARGETS)
endef

define RHASH_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D)/librhash \
		DESTDIR="$(STAGING_DIR)" $(RHASH_MAKE_OPTS) $(RHASH_INSTALL_TARGETS) \
		install-lib-headers
endef

ifeq ($(LINGMO_PACKAGE_RHASH_BIN),y)
define RHASH_INSTALL_TARGET_RHASH_BIN
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		DESTDIR="$(TARGET_DIR)" $(RHASH_MAKE_OPTS) build-install-binary
endef
endif

define RHASH_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D)/librhash \
		DESTDIR="$(TARGET_DIR)" $(RHASH_MAKE_OPTS) $(RHASH_INSTALL_TARGETS)
	$(RHASH_INSTALL_TARGET_RHASH_BIN)
endef

$(eval $(generic-package))
