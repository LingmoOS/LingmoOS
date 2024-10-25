################################################################################
#
# xlib_libXpm
#
################################################################################

XLIB_LIBXPM_VERSION = 3.5.17
XLIB_LIBXPM_SOURCE = libXpm-$(XLIB_LIBXPM_VERSION).tar.xz
XLIB_LIBXPM_SITE = https://xorg.freedesktop.org/archive/individual/lib
XLIB_LIBXPM_LICENSE = MIT
XLIB_LIBXPM_LICENSE_FILES = COPYING COPYRIGHT
XLIB_LIBXPM_CPE_ID_VENDOR = x.org
XLIB_LIBXPM_CPE_ID_PRODUCT = libxpm
XLIB_LIBXPM_INSTALL_STAGING = YES
XLIB_LIBXPM_DEPENDENCIES = xlib_libX11 xlib_libXext xlib_libXt xorgproto \
	$(if $(LINGMO_PACKAGE_LIBICONV),libiconv) \
	$(TARGET_NLS_DEPENDENCIES)
XLIB_LIBXPM_CONF_ENV = LIBS=$(TARGET_NLS_LIBS)
XLIB_LIBXPM_CONF_OPTS = --disable-open-zfile

ifeq ($(LINGMO_SYSTEM_ENABLE_NLS),)
XLIB_LIBXPM_CONF_ENV += ac_cv_search_gettext=no
endif

$(eval $(autotools-package))
