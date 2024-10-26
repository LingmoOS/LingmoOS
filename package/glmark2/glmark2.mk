################################################################################
#
# glmark2
#
################################################################################

GLMARK2_VERSION = 2023.01
GLMARK2_SITE = $(call github,glmark2,glmark2,$(GLMARK2_VERSION))
GLMARK2_LICENSE = GPL-3.0+, SGIv1
GLMARK2_LICENSE_FILES = COPYING COPYING.SGI
GLMARK2_DEPENDENCIES = host-pkgconf jpeg libegl libpng

ifeq ($(LINGMO_PACKAGE_GLMARK2_FLAVOR_DRM_GLESV2),y)
GLMARK2_DEPENDENCIES += libdrm libgbm libgles udev
GLMARK2_FLAVORS += drm-glesv2
endif

ifeq ($(LINGMO_PACKAGE_GLMARK2_FLAVOR_DRM_GL),y)
GLMARK2_DEPENDENCIES += libgl udev
GLMARK2_FLAVORS += drm-gl
endif

ifeq ($(LINGMO_PACKAGE_GLMARK2_FLAVOR_X11_GLESV2),y)
GLMARK2_DEPENDENCIES += libgles xlib_libX11
GLMARK2_FLAVORS += x11-glesv2
endif

ifeq ($(LINGMO_PACKAGE_GLMARK2_FLAVOR_X11_GL),y)
GLMARK2_DEPENDENCIES += libgl xlib_libX11
GLMARK2_FLAVORS += x11-gl
endif

ifeq ($(LINGMO_PACKAGE_GLMARK2_FLAVOR_WAYLAND_GLESV2),y)
GLMARK2_DEPENDENCIES += libgles wayland wayland-protocols
GLMARK2_FLAVORS += wayland-glesv2
endif

ifeq ($(LINGMO_PACKAGE_GLMARK2_FLAVOR_WAYLAND_GL),y)
GLMARK2_DEPENDENCIES += libgl wayland wayland-protocols
GLMARK2_FLAVORS += wayland-gl
endif

GLMARK2_CONF_OPTS = -Dflavors=$(subst $(space),$(comma),$(GLMARK2_FLAVORS))

$(eval $(meson-package))
