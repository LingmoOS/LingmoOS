################################################################################
#
# xapp_xdpyinfo
#
################################################################################

XAPP_XDPYINFO_VERSION = 1.3.4
XAPP_XDPYINFO_SOURCE = xdpyinfo-$(XAPP_XDPYINFO_VERSION).tar.xz
XAPP_XDPYINFO_SITE = https://xorg.freedesktop.org/archive/individual/app
XAPP_XDPYINFO_LICENSE = MIT
XAPP_XDPYINFO_LICENSE_FILES = COPYING
XAPP_XDPYINFO_CONF_OPTS = --without-xf86misc # not in BR
XAPP_XDPYINFO_DEPENDENCIES = xlib_libX11 xlib_libXext xlib_libXtst \
	$(if $(LINGMO_PACKAGE_XLIB_LIBXI),xlib_libXi) \
	$(if $(LINGMO_PACKAGE_XLIB_LIBXRENDER),xlib_libXrender) \
	$(if $(LINGMO_PACKAGE_XLIB_LIBXCOMPOSITE),xlib_libXcomposite) \
	$(if $(LINGMO_PACKAGE_XLIB_LIBXXF86VM),xlib_libXxf86vm)

ifeq ($(LINGMO_PACKAGE_XLIB_LIBXXF86DGA),y)
XAPP_XDPYINFO_DEPENDENCIES += xlib_libXxf86dga
else
XAPP_XDPYINFO_CONF_OPTS += --without-dga
endif

ifeq ($(LINGMO_PACKAGE_XLIB_LIBDMX),y)
XAPP_XDPYINFO_DEPENDENCIES += xlib_libdmx
else
XAPP_XDPYINFO_CONF_OPTS += --without-dmx
endif

ifeq ($(LINGMO_PACKAGE_XLIB_LIBXINERAMA),y)
XAPP_XDPYINFO_DEPENDENCIES += xlib_libXinerama
else
XAPP_XDPYINFO_CONF_OPTS += --without-xinerama
endif

$(eval $(autotools-package))
