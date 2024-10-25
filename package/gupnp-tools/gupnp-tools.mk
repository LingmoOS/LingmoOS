################################################################################
#
# gupnp-tools
#
################################################################################

GUPNP_TOOLS_VERSION_MAJOR = 0.12
GUPNP_TOOLS_VERSION = $(GUPNP_TOOLS_VERSION_MAJOR).1
GUPNP_TOOLS_SOURCE = gupnp-tools-$(GUPNP_TOOLS_VERSION).tar.xz
GUPNP_TOOLS_SITE = \
	https://download.gnome.org/sources/gupnp-tools/$(GUPNP_TOOLS_VERSION_MAJOR)
GUPNP_TOOLS_LICENSE = GPL-2.0+
GUPNP_TOOLS_LICENSE_FILES = COPYING
GUPNP_TOOLS_INSTALL_STAGING = YES
GUPNP_TOOLS_DEPENDENCIES = \
	host-pkgconf \
	libglib2 \
	libxml2 \
	gssdp \
	gupnp \
	libsoup3 \
	libgtk3 \
	$(TARGET_NLS_DEPENDENCIES)

GUPNP_TOOLS_LDFLAGS = $(TARGET_LDFLAGS) $(TARGET_NLS_LIBS)

ifeq ($(LINGMO_PACKAGE_GUPNP_AV),y)
GUPNP_TOOLS_CONF_OPTS += -Dav-tools=true
GUPNP_TOOLS_DEPENDENCIES += gupnp-av
else
GUPNP_TOOLS_CONF_OPTS += -Dav-tools=false
endif

ifeq ($(LINGMO_PACKAGE_GTKSOURCEVIEW),y)
GUPNP_TOOLS_DEPENDENCIES += gtksourceview
endif

$(eval $(meson-package))
