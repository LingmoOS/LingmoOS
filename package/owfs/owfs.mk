################################################################################
#
# owfs
#
################################################################################

OWFS_VERSION = 3.2p4
OWFS_SITE = https://github.com/owfs/owfs/releases/download/v$(OWFS_VERSION)
OWFS_DEPENDENCIES = host-pkgconf
OWFS_CONF_OPTS = --disable-owperl --without-perl5 --disable-owtcl --without-tcl

# owtcl license is declared in module/ownet/c/src/include/ow_functions.h
OWFS_LICENSE = GPL-2.0+, LGPL-2.0 (owtcl)
OWFS_LICENSE_FILES = COPYING COPYING.LIB
OWFS_INSTALL_STAGING = YES

# owfs PHP support is not PHP 7 compliant
# https://sourceforge.net/p/owfs/support-requests/32/
OWFS_CONF_OPTS += --disable-owphp --without-php

# Skip man pages processing
OWFS_CONF_ENV += ac_cv_path_SOELIM=true

ifeq ($(LINGMO_PACKAGE_LIBFUSE),y)
OWFS_DEPENDENCIES += libfuse
OWFS_CONF_OPTS += \
	--enable-owfs \
	--with-fuseinclude=$(STAGING_DIR)/usr/include \
	--with-fuselib=$(STAGING_DIR)/usr/lib
define OWFS_INSTALL_FUSE_INIT_SYSV
	$(INSTALL) -D -m 0755 $(OWFS_PKGDIR)/S60owfs \
		$(TARGET_DIR)/etc/init.d/S60owfs
endef
define OWFS_CREATE_MOUNTPOINT
	mkdir -p $(TARGET_DIR)/dev/1wire
endef
OWFS_POST_INSTALL_TARGET_HOOKS += OWFS_CREATE_MOUNTPOINT
else
OWFS_CONF_OPTS += --disable-owfs
endif

ifeq ($(LINGMO_PACKAGE_LIBFTDI1),y)
OWFS_CONF_OPTS += \
	--enable-ftdi \
	--with-libftdi-config=$(STAGING_DIR)/usr/bin/libftdi1-config
OWFS_DEPENDENCIES += libftdi1
else ifeq ($(LINGMO_PACKAGE_LIBFTDI),y)
OWFS_CONF_OPTS += \
	--enable-ftdi \
	--with-libftdi-config=$(STAGING_DIR)/usr/bin/libftdi-config
OWFS_DEPENDENCIES += libftdi
else
OWFS_CONF_OPTS += --disable-ftdi
endif

ifeq ($(LINGMO_PACKAGE_LIBUSB),y)
OWFS_CONF_OPTS += --enable-usb
OWFS_DEPENDENCIES += libusb
else
OWFS_CONF_OPTS += --disable-usb
endif

ifeq ($(LINGMO_PACKAGE_AVAHI),y)
OWFS_CONF_OPTS += --enable-avahi
OWFS_DEPENDENCIES += avahi
else
OWFS_CONF_OPTS += --disable-avahi
endif

# python2 only (https://github.com/owfs/owfs/pull/32)
OWFS_CONF_OPTS += --disable-owpython --without-python

ifeq ($(LINGMO_STATIC_LIBS),y)
# zeroconf support uses dlopen()
OWFS_CONF_OPTS += --disable-zero
endif

OWFS_MAKE = $(MAKE) $(OWFS_EXTRA_MAKE_OPTS)

define OWFS_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 $(OWFS_PKGDIR)/S55owserver \
		$(TARGET_DIR)/etc/init.d/S55owserver
	$(OWFS_INSTALL_FUSE_INIT_SYSV)
endef

$(eval $(autotools-package))
