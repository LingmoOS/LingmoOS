################################################################################
#
# libgsasl
#
################################################################################

LIBGSASL_VERSION = 1.10.0
LIBGSASL_SITE = $(LINGMO_GNU_MIRROR)/gsasl
LIBGSASL_LICENSE = LGPL-2.1+ (library), GPL-3.0+ (programs)
LIBGSASL_LICENSE_FILES = README COPYING.LIB COPYING
LIBGSASL_INSTALL_STAGING = YES

ifeq ($(LINGMO_PACKAGE_LIBGCRYPT),y)
LIBGSASL_CONF_OPTS += --with-libgcrypt-prefix=$(STAGING_DIR)/usr
LIBGSASL_DEPENDENCIES += libgcrypt
else
LIBGSASL_CONF_OPTS += --without-libgcrypt
endif

ifeq ($(LINGMO_PACKAGE_LIBIDN),y)
LIBGSASL_CONF_OPTS += --with-libidn-prefix=$(STAGING_DIR)/usr
LIBGSASL_DEPENDENCIES += libidn
else
LIBGSASL_CONF_OPTS += --without-stringprep
endif

$(eval $(autotools-package))
