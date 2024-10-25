################################################################################
#
# cups-filters
#
################################################################################

CUPS_FILTERS_VERSION = 1.28.17
CUPS_FILTERS_SITE = https://github.com/OpenPrinting/cups-filters/releases/download/$(CUPS_FILTERS_VERSION)
CUPS_FILTERS_LICENSE = GPL-2.0, GPL-2.0+, GPL-3.0, GPL-3.0+, LGPL-2, LGPL-2.1+, MIT, BSD-4-Clause
CUPS_FILTERS_LICENSE_FILES = COPYING
CUPS_FILTERS_CPE_ID_VENDOR = linuxfoundation

# 0001-beh-backend-Use-execv-instead-of-system-CVE-2023-24805.patch
CUPS_FILTERS_IGNORE_CVES += CVE-2023-24805

CUPS_FILTERS_DEPENDENCIES = cups libglib2 lcms2 qpdf fontconfig freetype jpeg

CUPS_FILTERS_CONF_OPTS = \
	--disable-mutool \
	--disable-foomatic \
	--disable-braille \
	--enable-imagefilters \
	--with-cups-config=$(STAGING_DIR)/usr/bin/cups-config \
	--with-sysroot=$(STAGING_DIR) \
	--with-pdftops=pdftops \
	--with-jpeg \
	--with-test-font-path=/dev/null \
	--without-rcdir

ifeq ($(LINGMO_PACKAGE_LIBPNG),y)
CUPS_FILTERS_CONF_OPTS += --with-png
CUPS_FILTERS_DEPENDENCIES += libpng
else
CUPS_FILTERS_CONF_OPTS += --without-png
endif

ifeq ($(LINGMO_PACKAGE_TIFF),y)
CUPS_FILTERS_CONF_OPTS += --with-tiff
CUPS_FILTERS_DEPENDENCIES += tiff
else
CUPS_FILTERS_CONF_OPTS += --without-tiff
endif

ifeq ($(LINGMO_PACKAGE_DBUS),y)
CUPS_FILTERS_CONF_OPTS += --enable-dbus
CUPS_FILTERS_DEPENDENCIES += dbus
else
CUPS_FILTERS_CONF_OPTS += --disable-dbus
endif

ifeq ($(LINGMO_PACKAGE_AVAHI_LIBAVAHI_CLIENT),y)
CUPS_FILTERS_DEPENDENCIES += avahi
CUPS_FILTERS_CONF_OPTS += --enable-avahi
else
CUPS_FILTERS_CONF_OPTS += --disable-avahi
endif

ifeq ($(LINGMO_PACKAGE_GHOSTSCRIPT),y)
CUPS_FILTERS_DEPENDENCIES += ghostscript
CUPS_FILTERS_CONF_OPTS += --enable-ghostscript
else
CUPS_FILTERS_CONF_OPTS += --disable-ghostscript
endif

ifeq ($(LINGMO_PACKAGE_IJS),y)
CUPS_FILTERS_DEPENDENCIES += ijs
CUPS_FILTERS_CONF_OPTS += --enable-ijs
else
CUPS_FILTERS_CONF_OPTS += --disable-ijs
endif

ifeq ($(LINGMO_PACKAGE_POPPLER),y)
CUPS_FILTERS_DEPENDENCIES += poppler
CUPS_FILTERS_CONF_OPTS += --enable-poppler
else
CUPS_FILTERS_CONF_OPTS += --disable-poppler
endif

ifeq ($(LINGMO_PACKAGE_LIBEXIF),y)
CUPS_FILTERS_CONF_OPTS += --enable-exif
CUPS_FILTERS_DEPENDENCIES += libexif
else
CUPS_FILTERS_CONF_OPTS += --disable-exif
endif

define CUPS_FILTERS_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 package/cups-filters/S82cups-browsed \
		$(TARGET_DIR)/etc/init.d/S82cups-browsed
endef

define CUPS_FILTERS_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0755 $(@D)/utils/cups-browsed.service \
		$(TARGET_DIR)/usr/lib/systemd/system/cups-browsed.service
endef

$(eval $(autotools-package))
