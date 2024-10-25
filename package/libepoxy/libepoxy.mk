################################################################################
#
# libepoxy
#
################################################################################

LIBEPOXY_VERSION_MAJOR = 1.5
LIBEPOXY_VERSION = $(LIBEPOXY_VERSION_MAJOR).10
LIBEPOXY_SITE = https://download.gnome.org/sources/libepoxy/$(LIBEPOXY_VERSION_MAJOR)
LIBEPOXY_SOURCE = libepoxy-$(LIBEPOXY_VERSION).tar.xz
LIBEPOXY_INSTALL_STAGING = YES
LIBEPOXY_DEPENDENCIES = host-pkgconf xutil_util-macros
LIBEPOXY_LICENSE = MIT
LIBEPOXY_LICENSE_FILES = COPYING
LIBEPOXY_CONF_OPTS += -Ddocs=false -Dtests=false

ifeq ($(LINGMO_PACKAGE_HAS_LIBEGL),y)
LIBEPOXY_CONF_OPTS += -Degl=yes
LIBEPOXY_DEPENDENCIES += libegl
else
LIBEPOXY_CONF_OPTS += -Degl=no
endif

ifeq ($(LINGMO_PACKAGE_HAS_LIBGL)$(LINGMO_PACKAGE_XLIB_LIBX11),yy)
LIBEPOXY_CONF_OPTS += -Dglx=yes -Dx11=true
LIBEPOXY_DEPENDENCIES += libgl xlib_libX11
else
LIBEPOXY_CONF_OPTS += -Dglx=no -Dx11=false
endif

$(eval $(meson-package))
