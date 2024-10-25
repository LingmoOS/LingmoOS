################################################################################
#
# gconf
#
################################################################################

GCONF_VERSION = 3.2.6
GCONF_SOURCE = GConf-$(GCONF_VERSION).tar.xz
GCONF_SITE = https://download.gnome.org/sources/GConf/3.2
GCONF_CONF_OPTS = --disable-orbit
GCONF_DEPENDENCIES = dbus dbus-glib libglib2 libxml2 \
	host-intltool $(TARGET_NLS_DEPENDENCIES)
GCONF_LICENSE = LGPL-2.0+
GCONF_LICENSE_FILES = COPYING

ifeq ($(LINGMO_PACKAGE_GOBJECT_INTROSPECTION),y)
GCONF_CONF_OPTS += --enable-introspection
GCONF_DEPENDENCIES += gobject-introspection
else
GCONF_CONF_OPTS += --disable-introspection
endif

$(eval $(autotools-package))
