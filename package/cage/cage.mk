################################################################################
#
# cage
#
################################################################################

CAGE_VERSION = 0.1.5
CAGE_SITE = https://github.com/Hjdskes/cage/releases/download/v$(CAGE_VERSION)
CAGE_LICENSE = MIT
CAGE_LICENSE_FILES = LICENSE
CAGE_DEPENDENCIES = host-pkgconf wlroots
CAGE_CONF_OPTS = -Dman-pages=disabled

ifeq ($(LINGMO_PACKAGE_WLROOTS_X11),y)
CAGE_CONF_OPTS += -Dxwayland=true
else
CAGE_CONF_OPTS += -Dxwayland=false
endif

$(eval $(meson-package))
