################################################################################
#
# libucl
#
################################################################################

LIBUCL_VERSION = 0.8.2
LIBUCL_SITE = $(call github,vstakhov,libucl,$(LIBUCL_VERSION))
LIBUCL_INSTALL_STAGING = YES
LIBUCL_AUTORECONF = YES
LIBUCL_LICENSE = BSD-2-Clause
LIBUCL_LICENSE_FILES = COPYING
LIBUCL_DEPENDENCIES = host-pkgconf

ifeq ($(LINGMO_PACKAGE_LIBCURL),y)
LIBUCL_DEPENDENCIES += libcurl
LIBUCL_CONF_OPTS += --enable-urls
else
LIBUCL_CONF_OPTS += --disable-urls
endif

$(eval $(autotools-package))
