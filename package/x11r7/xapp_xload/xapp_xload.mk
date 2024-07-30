################################################################################
#
# xapp_xload
#
################################################################################

XAPP_XLOAD_VERSION = 1.1.4
XAPP_XLOAD_SOURCE = xload-$(XAPP_XLOAD_VERSION).tar.xz
XAPP_XLOAD_SITE = http://xorg.freedesktop.org/releases/individual/app
XAPP_XLOAD_LICENSE = MIT
XAPP_XLOAD_LICENSE_FILES = COPYING
XAPP_XLOAD_DEPENDENCIES = xlib_libXaw $(TARGET_NLS_DEPENDENCIES)
XAPP_XLOAD_CONF_OPTS = --with-appdefaultdir=/usr/share/X11/app-defaults
XAPP_XLOAD_CONF_ENV = LIBS=$(TARGET_NLS_LIBS)

ifeq ($(LINGMO_TOOLCHAIN_USES_MUSL),y)
# musl doesn't have rwhod.h, but xload can replace it with stubs
XAPP_XLOAD_CONF_OPTS += CFLAGS="$(TARGET_CFLAGS) -DRLOADSTUB"
endif

ifeq ($(LINGMO_SYSTEM_ENABLE_NLS),)
XAPP_XLOAD_CONF_ENV += ac_cv_search_gettext=no
endif

$(eval $(autotools-package))
