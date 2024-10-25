################################################################################
#
# leptonica
#
################################################################################

LEPTONICA_VERSION = 1.83.1
LEPTONICA_SITE = https://github.com/DanBloomberg/leptonica/releases/download/$(LEPTONICA_VERSION)
LEPTONICA_LICENSE = BSD-2-Clause
LEPTONICA_LICENSE_FILES = leptonica-license.txt
LEPTONICA_CPE_ID_VENDOR = leptonica
LEPTONICA_INSTALL_STAGING = YES
LEPTONICA_DEPENDENCIES = host-pkgconf

LEPTONICA_CONF_OPTS += --disable-programs

ifeq ($(LINGMO_TOOLCHAIN_HAS_GCC_BUG_101737),y)
LEPTONICA_CONF_OPTS += CFLAGS="$(TARGET_CFLAGS) -O0"
endif

ifeq ($(LINGMO_PACKAGE_GIFLIB),y)
LEPTONICA_DEPENDENCIES += giflib
LEPTONICA_CONF_OPTS += --with-giflib
else
LEPTONICA_CONF_OPTS += --without-giflib
endif

ifeq ($(LINGMO_PACKAGE_JPEG),y)
LEPTONICA_DEPENDENCIES += jpeg
LEPTONICA_CONF_OPTS += --with-jpeg
else
LEPTONICA_CONF_OPTS += --without-jpeg
endif

ifeq ($(LINGMO_PACKAGE_LIBPNG),y)
LEPTONICA_DEPENDENCIES += libpng
LEPTONICA_CONF_OPTS += --with-libpng
else
LEPTONICA_CONF_OPTS += --without-libpng
endif

ifeq ($(LINGMO_PACKAGE_OPENJPEG),y)
LEPTONICA_DEPENDENCIES += openjpeg
LEPTONICA_CONF_OPTS += --with-libopenjpeg
else
LEPTONICA_CONF_OPTS += --without-libopenjpeg
endif

ifeq ($(LINGMO_PACKAGE_TIFF),y)
LEPTONICA_DEPENDENCIES += tiff
LEPTONICA_CONF_OPTS += --with-libtiff
else
LEPTONICA_CONF_OPTS += --without-libtiff
endif

ifeq ($(LINGMO_PACKAGE_WEBP),y)
LEPTONICA_DEPENDENCIES += webp
LEPTONICA_CONF_OPTS += --with-libwebp
ifeq ($(LINGMO_PACKAGE_WEBP_DEMUX)$(LINGMO_PACKAGE_WEBP_MUX),yy)
LEPTONICA_CONF_OPTS += --with-libwebpmux
else
LEPTONICA_CONF_OPTS += --without-libwebpmux
endif
else
LEPTONICA_CONF_OPTS += \
	--without-libwebp \
	--without-libwebpmux
endif

ifeq ($(LINGMO_PACKAGE_ZLIB),y)
LEPTONICA_DEPENDENCIES += zlib
LEPTONICA_CONF_OPTS += --with-zlib
else
LEPTONICA_CONF_OPTS += --without-zlib
endif

$(eval $(autotools-package))
