################################################################################
#
# sane-backends
#
################################################################################

SANE_BACKENDS_VERSION = 1.1.1
SANE_BACKENDS_SITE = \
	https://gitlab.com/sane-project/backends/uploads/7d30fab4e115029d91027b6a58d64b43
SANE_BACKENDS_CONFIG_SCRIPTS = sane-config
SANE_BACKENDS_LICENSE = GPL-2.0+
SANE_BACKENDS_LICENSE_FILES = COPYING
SANE_BACKENDS_CPE_ID_VALID = YES
SANE_BACKENDS_INSTALL_STAGING = YES

SANE_BACKENDS_CONF_OPTS = \
	$(if $(LINGMO_TOOLCHAIN_HAS_THREADS),--enable-pthread,--disable-pthread)

ifeq ($(LINGMO_INIT_SYSTEMD),y)
SANE_BACKENDS_CONF_OPTS += --with-systemd
SANE_BACKENDS_DEPENDENCIES += systemd
else
SANE_BACKENDS_CONF_OPTS += --without-systemd
endif

ifeq ($(LINGMO_PACKAGE_LIBUSB),y)
SANE_BACKENDS_DEPENDENCIES += libusb
SANE_BACKENDS_CONF_OPTS += --with-usb
else
SANE_BACKENDS_CONF_OPTS += --without-usb
endif

ifeq ($(LINGMO_PACKAGE_JPEG),y)
SANE_BACKENDS_DEPENDENCIES += jpeg
endif

ifeq ($(LINGMO_PACKAGE_TIFF),y)
SANE_BACKENDS_DEPENDENCIES += tiff
endif

ifeq ($(LINGMO_PACKAGE_LIBV4L),y)
SANE_BACKENDS_DEPENDENCIES += libv4l
endif

ifeq ($(LINGMO_PACKAGE_AVAHI)$(LINGMO_PACKAGE_DBUS)$(LINGMO_PACKAGE_LIBGLIB2),yyy)
SANE_BACKENDS_DEPENDENCIES += avahi
SANE_BACKENDS_CONF_OPTS += --with-avahi
endif

ifeq ($(LINGMO_PACKAGE_NETSNMP),y)
SANE_BACKENDS_CONF_ENV += ac_cv_path_SNMP_CONFIG_PATH=$(STAGING_DIR)/usr/bin/net-snmp-config
SANE_BACKENDS_DEPENDENCIES += netsnmp
else
SANE_BACKENDS_CONF_OPTS += --without-snmp
endif

ifeq ($(LINGMO_PACKAGE_LIBCURL),y)
SANE_BACKENDS_DEPENDENCIES += libcurl
SANE_BACKENDS_CONF_OPTS += --with-libcurl
else
SANE_BACKENDS_CONF_OPTS += --without-libcurl
endif

ifeq ($(LINGMO_PACKAGE_POPPLER)$(LINGMO_PACKAGE_CAIRO)$(LINGMO_PACKAGE_LIBGLIB2),yyy)
SANE_BACKENDS_DEPENDENCIES += poppler libglib2
SANE_BACKENDS_CONF_OPTS += --with-poppler-glib
else
SANE_BACKENDS_CONF_OPTS += --without-poppler-glib
endif

ifeq ($(LINGMO_PACKAGE_LIBXML2),y)
SANE_BACKENDS_DEPENDENCIES += libxml2
SANE_BACKENDS_CONF_OPTS += --with-usb-record-replay
else
SANE_BACKENDS_CONF_OPTS += --without-usb-record-replay
endif

define SANE_BACKENDS_DISABLE_DOCS
	$(SED) 's/ doc//' $(@D)/Makefile
endef

SANE_BACKENDS_POST_CONFIGURE_HOOKS += SANE_BACKENDS_DISABLE_DOCS

define SANE_BACKENDS_USERS
	saned -1 saned -1 * /etc/sane.d - - Saned User
endef

define SANE_BACKENDS_INSTALL_INIT_SYSTEMD
	$(INSTALL) -m 0644 -D package/sane-backends/saned.socket \
		$(TARGET_DIR)/usr/lib/systemd/system/saned.socket
	$(INSTALL) -m 0644 -D package/sane-backends/saned@.service \
		$(TARGET_DIR)/usr/lib/systemd/system/saned@.service
endef

$(eval $(autotools-package))
