################################################################################
#
# libidn2
#
################################################################################

LIBIDN2_VERSION = 2.3.4
LIBIDN2_SITE = $(LINGMO_GNU_MIRROR)/libidn
LIBIDN2_LICENSE = GPL-2.0+ or LGPL-3.0+ (library)
LIBIDN2_LICENSE_FILES = COPYING COPYINGv2 COPYING.LESSERv3 COPYING.unicode
LIBIDN2_CPE_ID_VENDOR = gnu
LIBIDN2_DEPENDENCIES = \
	host-pkgconf \
	$(TARGET_NLS_DEPENDENCIES) \
	$(if $(LINGMO_PACKAGE_LIBICONV),libiconv)
LIBIDN2_CONF_OPTS = --without-included-libunistring
LIBIDN2_INSTALL_STAGING = YES

ifeq ($(LINGMO_PACKAGE_LIBUNISTRING),y)
LIBIDN2_DEPENDENCIES += libunistring
LIBIDN2_CONF_OPTS += --with-libunistring-prefix=$(STAGING_DIR)/usr/lib
endif

ifeq ($(LINGMO_PACKAGE_LIBIDN2_BINARY),)
define LIBIDN2_REMOVE_BINARY
	rm -f $(TARGET_DIR)/usr/bin/idn2
endef
LIBIDN2_POST_INSTALL_TARGET_HOOKS += LIBIDN2_REMOVE_BINARY
else
LIBIDN2_LICENSE += , GPL-3.0+ (program)
endif

$(eval $(autotools-package))
