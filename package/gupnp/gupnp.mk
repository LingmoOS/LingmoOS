################################################################################
#
# gupnp
#
################################################################################

GUPNP_VERSION_MAJOR = 1.6
GUPNP_VERSION = $(GUPNP_VERSION_MAJOR).6
GUPNP_SOURCE = gupnp-$(GUPNP_VERSION).tar.xz
GUPNP_SITE = https://download.gnome.org/sources/gupnp/$(GUPNP_VERSION_MAJOR)
GUPNP_LICENSE = LGPL-2.1+
GUPNP_LICENSE_FILES = COPYING
GUPNP_CPE_ID_VENDOR = gnome
GUPNP_INSTALL_STAGING = YES
GUPNP_DEPENDENCIES = host-pkgconf libglib2 libxml2 gssdp util-linux
GUPNP_CONF_OPTS = -Dexamples=false

ifeq ($(LINGMO_PACKAGE_GOBJECT_INTROSPECTION),y)
GUPNP_CONF_OPTS += -Dintrospection=true -Dvapi=true
GUPNP_DEPENDENCIES += host-vala gobject-introspection
else
GUPNP_CONF_OPTS += -Dintrospection=false -Dvapi=false
endif

$(eval $(meson-package))
