################################################################################
#
# json-glib
#
################################################################################

JSON_GLIB_VERSION_MAJOR = 1.8
JSON_GLIB_VERSION = $(JSON_GLIB_VERSION_MAJOR).0
JSON_GLIB_SITE = https://download.gnome.org/sources/json-glib/$(JSON_GLIB_VERSION_MAJOR)
JSON_GLIB_SOURCE = json-glib-$(JSON_GLIB_VERSION).tar.xz
JSON_GLIB_LICENSE = LGPL-2.1+
JSON_GLIB_LICENSE_FILES = COPYING
JSON_GLIB_INSTALL_STAGING = YES
JSON_GLIB_CONF_OPTS = -Dgtk_doc=disabled -Dtests=false

JSON_GLIB_DEPENDENCIES = \
	$(TARGET_NLS_DEPENDENCIES) \
	host-pkgconf \
	libglib2

ifeq ($(LINGMO_PACKAGE_GOBJECT_INTROSPECTION),y)
JSON_GLIB_CONF_OPTS += -Dintrospection=enabled
JSON_GLIB_DEPENDENCIES += gobject-introspection
else
JSON_GLIB_CONF_OPTS += -Dintrospection=disabled
endif

ifeq ($(LINGMO_SYSTEM_ENABLE_NLS),y)
JSON_GLIB_CONF_OPTS += -Dnls=enabled
else
JSON_GLIB_CONF_OPTS += -Dnls=disabled
endif

JSON_GLIB_LDFLAGS = $(TARGET_LDFLAGS) $(TARGET_NLS_LIBS)

$(eval $(meson-package))
