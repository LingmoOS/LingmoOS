################################################################################
#
# gupnp-av
#
################################################################################

GUPNP_AV_VERSION_MAJOR = 0.14
GUPNP_AV_VERSION = $(GUPNP_AV_VERSION_MAJOR).1
GUPNP_AV_SOURCE = gupnp-av-$(GUPNP_AV_VERSION).tar.xz
GUPNP_AV_SITE = https://download.gnome.org/sources/gupnp-av/$(GUPNP_AV_VERSION_MAJOR)
GUPNP_AV_LICENSE = LGPL-2.1+
GUPNP_AV_LICENSE_FILES = COPYING
GUPNP_AV_INSTALL_STAGING = YES
GUPNP_AV_DEPENDENCIES = host-pkgconf libglib2 libxml2 gupnp

ifeq ($(LINGMO_PACKAGE_GOBJECT_INTROSPECTION),y)
GUPNP_AV_CONF_OPTS += -Dintrospection=true -Dvapi=true
GUPNP_AV_DEPENDENCIES += host-vala gobject-introspection
else
GUPNP_AV_CONF_OPTS += -Dintrospection=false -Dvapi=false
endif

$(eval $(meson-package))
