################################################################################
#
# gd
#
################################################################################

GD_VERSION = 2.3.3
GD_SOURCE = libgd-$(GD_VERSION).tar.xz
GD_SITE = https://github.com/libgd/libgd/releases/download/gd-$(GD_VERSION)
GD_INSTALL_STAGING = YES
GD_LICENSE = GD license
GD_LICENSE_FILES = COPYING
GD_CPE_ID_VENDOR = libgd
GD_CPE_ID_PRODUCT = libgd
GD_CONF_OPTS = --without-x --disable-rpath --disable-werror
GD_DEPENDENCIES = host-pkgconf

# gd forgets to link utilities with -pthread even though it uses
# pthreads, causing linking errors with static linking
ifeq ($(LINGMO_TOOLCHAIN_HAS_THREADS),y)
GD_CONF_ENV += LDFLAGS="$(TARGET_LDFLAGS) -pthread"
endif

ifeq ($(LINGMO_PACKAGE_FONTCONFIG),y)
GD_DEPENDENCIES += fontconfig
GD_CONF_OPTS += --with-fontconfig
endif

ifeq ($(LINGMO_PACKAGE_FREETYPE),y)
GD_DEPENDENCIES += freetype
GD_CONF_OPTS += --with-freetype=$(STAGING_DIR)/usr
else
GD_CONF_OPTS += --without-freetype
endif

ifeq ($(LINGMO_PACKAGE_LIBICONV),y)
GD_DEPENDENCIES += libiconv
# not strictly needed for gd, but ensures -liconv ends up in gdlib.pc
GD_CONF_ENV += LIBS_PRIVATES="-liconv"
endif

ifeq ($(LINGMO_PACKAGE_JPEG),y)
GD_DEPENDENCIES += jpeg
GD_CONF_OPTS += --with-jpeg
endif

ifeq ($(LINGMO_PACKAGE_LIBHEIF),y)
GD_DEPENDENCIES += libheif
GD_CONF_OPTS += --with-heif
else
GD_CONF_OPTS += --without-heif
endif

ifeq ($(LINGMO_PACKAGE_LIBPNG),y)
GD_DEPENDENCIES += libpng
GD_CONF_OPTS += --with-png
else
GD_CONF_OPTS += --without-png
endif

ifeq ($(LINGMO_PACKAGE_WEBP),y)
GD_DEPENDENCIES += webp
GD_CONF_OPTS += --with-webp
else
GD_CONF_OPTS += --without-webp
endif

ifeq ($(LINGMO_PACKAGE_TIFF),y)
GD_DEPENDENCIES += tiff
GD_CONF_OPTS += --with-tiff
else
GD_CONF_OPTS += --without-tiff
endif

ifeq ($(LINGMO_PACKAGE_XLIB_LIBXPM),y)
GD_DEPENDENCIES += xlib_libXpm
GD_CONF_OPTS += --with-xpm
endif

ifeq ($(LINGMO_PACKAGE_ZLIB),y)
GD_DEPENDENCIES += zlib
endif

GD_TOOLS_$(LINGMO_PACKAGE_GD_ANNOTATE)	+= annotate
GD_TOOLS_$(LINGMO_PACKAGE_GD_BDFTOGD)	+= bdftogd
GD_TOOLS_$(LINGMO_PACKAGE_GD_GD2COPYPAL)	+= gd2copypal
GD_TOOLS_$(LINGMO_PACKAGE_GD_GD2TOGIF)	+= gd2togif
GD_TOOLS_$(LINGMO_PACKAGE_GD_GD2TOPNG)	+= gd2topng
GD_TOOLS_$(LINGMO_PACKAGE_GD_GDCMPGIF)	+= gdcmpgif
GD_TOOLS_$(LINGMO_PACKAGE_GD_GDPARTTOPNG)	+= gdparttopng
GD_TOOLS_$(LINGMO_PACKAGE_GD_GDTOPNG)	+= gdtopng
GD_TOOLS_$(LINGMO_PACKAGE_GD_GIFTOGD2)	+= giftogd2
GD_TOOLS_$(LINGMO_PACKAGE_GD_PNGTOGD)	+= pngtogd
GD_TOOLS_$(LINGMO_PACKAGE_GD_PNGTOGD2)	+= pngtogd2
GD_TOOLS_$(LINGMO_PACKAGE_GD_WEBPNG)	+= webpng

define GD_REMOVE_TOOLS
	rm -f $(addprefix $(TARGET_DIR)/usr/bin/,$(GD_TOOLS_))
endef

GD_POST_INSTALL_TARGET_HOOKS += GD_REMOVE_TOOLS

$(eval $(autotools-package))
