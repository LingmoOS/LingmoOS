################################################################################
#
# tinycbor
#
################################################################################

TINYCBOR_VERSION = 0.6.0
TINYCBOR_SITE = $(call github,intel,tinycbor,v$(TINYCBOR_VERSION))
TINYCBOR_LICENSE = MIT
TINYCBOR_LICENSE_FILES = LICENSE

TINYCBOR_DEPENDENCIES = host-pkgconf
TINYCBOR_INSTALL_STAGING = YES

ifeq ($(LINGMO_PACKAGE_CJSON),y)
TINYCBOR_DEPENDENCIES += cjson
endif

TINYCBOR_MAKE_OPTS = $(TARGET_CONFIGURE_OPTS) DISABLE_WERROR=1 V=1 prefix=/usr

ifeq ($(LINGMO_STATIC_LIBS),y)
TINYCBOR_MAKE_OPTS += BUILD_STATIC=1 BUILD_SHARED=0
else ifeq ($(LINGMO_SHARED_STATIC_LIBS),y)
TINYCBOR_MAKE_OPTS += BUILD_STATIC=1 BUILD_SHARED=1
else ifeq ($(LINGMO_SHARED_LIBS),y)
TINYCBOR_MAKE_OPTS += BUILD_STATIC=0 BUILD_SHARED=1
endif

# disabled parallel build because of build failures while
# producing the .config file
define TINYCBOR_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE1) $(TINYCBOR_MAKE_OPTS) -C $(@D)
endef

define TINYCBOR_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TINYCBOR_MAKE_OPTS) -C $(@D) \
		DESTDIR=$(STAGING_DIR) install
endef

define TINYCBOR_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TINYCBOR_MAKE_OPTS) -C $(@D) \
		DESTDIR=$(TARGET_DIR) install
endef

$(eval $(generic-package))
