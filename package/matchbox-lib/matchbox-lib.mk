################################################################################
#
# matchbox-lib
#
################################################################################

MATCHBOX_LIB_VERSION = 1.12
MATCHBOX_LIB_SOURCE = libmatchbox-$(MATCHBOX_LIB_VERSION).tar.bz2
MATCHBOX_LIB_SITE = http://git.yoctoproject.org/cgit/cgit.cgi/libmatchbox/snapshot
MATCHBOX_LIB_LICENSE = LGPL-2.1+
MATCHBOX_LIB_LICENSE_FILES = COPYING
# From git
MATCHBOX_LIB_AUTORECONF = YES
MATCHBOX_LIB_INSTALL_STAGING = YES
MATCHBOX_LIB_DEPENDENCIES = host-pkgconf xlib_libXext
MATCHBOX_LIB_CONF_OPTS = --disable-doxygen-docs
MATCHBOX_LIB_CONF_ENV = LIBS="-lX11"

define MATCHBOX_LIB_POST_INSTALL_FIXES
	$(SED) 's:-I$(STAGING_DIR)/:-I/:g' \
		-e 's:-I/usr/include\( \|$$\)::g' \
		$(STAGING_DIR)/usr/lib/pkgconfig/libmb.pc
endef

MATCHBOX_LIB_POST_INSTALL_STAGING_HOOKS += MATCHBOX_LIB_POST_INSTALL_FIXES

ifeq ($(LINGMO_PACKAGE_XLIB_LIBXCOMPOSITE),y)
ifeq ($(LINGMO_PACKAGE_XLIB_LIBXPM),y)
MATCHBOX_LIB_DEPENDENCIES += xlib_libXpm
endif
endif

ifeq ($(LINGMO_PACKAGE_JPEG),y)
MATCHBOX_LIB_CONF_OPTS += --enable-jpeg
MATCHBOX_LIB_DEPENDENCIES += jpeg
else
MATCHBOX_LIB_CONF_OPTS += --disable-jpeg
endif

ifeq ($(LINGMO_PACKAGE_LIBPNG),y)
MATCHBOX_LIB_CONF_OPTS += --enable-png
MATCHBOX_LIB_DEPENDENCIES += libpng
else
MATCHBOX_LIB_CONF_OPTS += --disable-png
endif

ifeq ($(LINGMO_PACKAGE_PANGO)$(LINGMO_PACKAGE_XLIB_LIBXFT)$(LINGMO_PACKAGE_XLIB_LIBXRENDER),yyy)
MATCHBOX_LIB_CONF_OPTS += --enable-pango
MATCHBOX_LIB_DEPENDENCIES += pango
else
MATCHBOX_LIB_CONF_OPTS += --disable-pango
endif

ifeq ($(LINGMO_PACKAGE_XLIB_LIBXFT),y)
MATCHBOX_LIB_CONF_OPTS += --enable-xft
MATCHBOX_LIB_DEPENDENCIES += xlib_libXft
else
MATCHBOX_LIB_CONF_OPTS += --disable-xft
endif

$(eval $(autotools-package))
