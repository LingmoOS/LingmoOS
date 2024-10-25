################################################################################
#
# libxml2
#
################################################################################

LIBXML2_VERSION_MAJOR = 2.12
LIBXML2_VERSION = $(LIBXML2_VERSION_MAJOR).6
LIBXML2_SOURCE = libxml2-$(LIBXML2_VERSION).tar.xz
LIBXML2_SITE = \
	https://download.gnome.org/sources/libxml2/$(LIBXML2_VERSION_MAJOR)
LIBXML2_INSTALL_STAGING = YES
LIBXML2_LICENSE = MIT
LIBXML2_LICENSE_FILES = Copyright
LIBXML2_CPE_ID_VENDOR = xmlsoft
LIBXML2_CONFIG_SCRIPTS = xml2-config

# relocation truncated to fit: R_68K_GOT16O
ifeq ($(LINGMO_m68k_cf),y)
LIBXML2_CONF_ENV += CFLAGS="$(TARGET_CFLAGS) -mxgot"
endif

LIBXML2_CONF_OPTS = --with-gnu-ld --without-debug

HOST_LIBXML2_DEPENDENCIES = host-pkgconf
LIBXML2_DEPENDENCIES = host-pkgconf

HOST_LIBXML2_CONF_OPTS = --without-zlib --without-lzma

ifeq ($(LINGMO_PACKAGE_PYTHON3),y)
LIBXML2_DEPENDENCIES += python3
LIBXML2_CONF_OPTS += --with-python
else
LIBXML2_CONF_OPTS += --without-python
endif

ifeq ($(LINGMO_PACKAGE_HOST_PYTHON3),y)
HOST_LIBXML2_DEPENDENCIES += host-python3
HOST_LIBXML2_CONF_OPTS += --with-python
else
HOST_LIBXML2_CONF_OPTS += --without-python
endif

ifeq ($(LINGMO_PACKAGE_ICU),y)
LIBXML2_DEPENDENCIES += icu
LIBXML2_CONF_OPTS += --with-icu
else
LIBXML2_CONF_OPTS += --without-icu
endif

ifeq ($(LINGMO_PACKAGE_ZLIB),y)
LIBXML2_DEPENDENCIES += zlib
LIBXML2_CONF_OPTS += --with-zlib=$(STAGING_DIR)/usr
else
LIBXML2_CONF_OPTS += --without-zlib
endif

ifeq ($(LINGMO_PACKAGE_XZ),y)
LIBXML2_DEPENDENCIES += xz
LIBXML2_CONF_OPTS += --with-lzma
else
LIBXML2_CONF_OPTS += --without-lzma
endif

LIBXML2_DEPENDENCIES += $(if $(LINGMO_PACKAGE_LIBICONV),libiconv)

ifeq ($(LINGMO_ENABLE_LOCALE)$(LINGMO_PACKAGE_LIBICONV),y)
LIBXML2_CONF_OPTS += --with-iconv
else
LIBXML2_CONF_OPTS += --without-iconv
endif

define LIBXML2_CLEANUP_XML2CONF
	rm -f $(TARGET_DIR)/usr/lib/xml2Conf.sh
endef
LIBXML2_POST_INSTALL_TARGET_HOOKS += LIBXML2_CLEANUP_XML2CONF

$(eval $(autotools-package))
$(eval $(host-autotools-package))

# libxml2 for the host
LIBXML2_HOST_BINARY = $(HOST_DIR)/bin/xmllint
