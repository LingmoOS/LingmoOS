################################################################################
#
# libsoup3
#
################################################################################

LIBSOUP3_VERSION_MAJOR = 3.4
LIBSOUP3_VERSION = $(LIBSOUP3_VERSION_MAJOR).4
LIBSOUP3_SOURCE = libsoup-$(LIBSOUP3_VERSION).tar.xz
LIBSOUP3_SITE = https://download.gnome.org/sources/libsoup/$(LIBSOUP3_VERSION_MAJOR)
LIBSOUP3_LICENSE = LGPL-2.0+
LIBSOUP3_LICENSE_FILES = COPYING
LIBSOUP3_CPE_ID_VENDOR = gnome
LIBSOUP3_INSTALL_STAGING = YES
LIBSOUP3_DEPENDENCIES = \
	host-intltool \
	host-libglib2 \
	host-pkgconf \
	libglib2 \
	libpsl \
	nghttp2 \
	sqlite \
	$(TARGET_NLS_DEPENDENCIES)

LIBSOUP3_LDFLAGS = $(TARGET_LDFLAGS) $(TARGET_NLS_LIBS)

LIBSOUP3_CONF_OPTS = \
	-Dntlm=disabled \
	-Dsysprof=disabled \
	-Dtests=false \
	-Dtls_check=false

ifeq ($(LINGMO_PACKAGE_BROTLI),y)
LIBSOUP3_CONF_OPTS += -Dbrotli=enabled
LIBSOUP3_DEPENDENCIES += brotli
else
LIBSOUP3_CONF_OPTS += -Dbrotli=disabled
endif

ifeq ($(LINGMO_PACKAGE_GOBJECT_INTROSPECTION),y)
LIBSOUP3_CONF_OPTS += -Dintrospection=enabled -Dvapi=enabled
LIBSOUP3_DEPENDENCIES += host-vala gobject-introspection
else
LIBSOUP3_CONF_OPTS += -Dintrospection=disabled -Dvapi=disabled
endif

ifeq ($(LINGMO_PACKAGE_LIBKRB5),y)
LIBSOUP3_CONF_OPTS += \
	-Dgssapi=enabled \
	-Dkrb5_config=$(STAGING_DIR)/usr/bin/krb5-config
LIBSOUP3_DEPENDENCIES += libkrb5
else
LIBSOUP3_CONF_OPTS += -Dgssapi=disabled
endif

$(eval $(meson-package))
