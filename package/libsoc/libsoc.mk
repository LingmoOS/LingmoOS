################################################################################
#
# libsoc
#
################################################################################

LIBSOC_VERSION = 0.8.2
LIBSOC_SITE = $(call github,jackmitch,libsoc,$(LIBSOC_VERSION))
LIBSOC_LICENSE = LGPL-2.1
LIBSOC_LICENSE_FILES = LICENCE
LIBSOC_AUTORECONF = YES
LIBSOC_INSTALL_STAGING = YES
LIBSOC_DEPENDENCIES = host-pkgconf

# Install Python 3 bindings
ifeq ($(LINGMO_PACKAGE_PYTHON3),y)
LIBSOC_DEPENDENCIES += python3
LIBSOC_CONF_OPTS += --enable-python=3
else
LIBSOC_CONF_OPTS += --disable-python
endif

$(eval $(autotools-package))
