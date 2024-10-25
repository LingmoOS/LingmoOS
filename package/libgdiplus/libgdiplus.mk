################################################################################
#
# libgdiplus
#
################################################################################

LIBGDIPLUS_VERSION = 6.1
LIBGDIPLUS_SITE = https://download.mono-project.com/sources/libgdiplus

LIBGDIPLUS_LICENSE = MIT
LIBGDIPLUS_LICENSE_FILES = LICENSE
LIBGDIPLUS_CPE_ID_VENDOR = mono-project

LIBGDIPLUS_INSTALL_STAGING = YES

# github tarball doesn't have configure
LIBGDIPLUS_AUTORECONF = YES

LIBGDIPLUS_DEPENDENCIES = xlib_libXft libglib2 cairo libpng host-pkgconf

ifeq ($(LINGMO_PACKAGE_GIFLIB),y)
LIBGDIPLUS_CONF_OPTS += --with-libgif
LIBGDIPLUS_DEPENDENCIES += giflib
else
LIBGDIPLUS_CONF_OPTS += --without-libgif
endif

# there is a bug in the configure script that enables pango support
# when passing --without-pango, so let's just not use it
ifeq ($(LINGMO_PACKAGE_PANGO),y)
LIBGDIPLUS_CONF_OPTS += --with-pango
LIBGDIPLUS_DEPENDENCIES += pango
endif

ifeq ($(LINGMO_PACKAGE_LIBEXIF),y)
LIBGDIPLUS_CONF_OPTS += --with-libexif
LIBGDIPLUS_DEPENDENCIES += libexif
else
LIBGDIPLUS_CONF_OPTS += --without-libexif
endif

ifeq ($(LINGMO_PACKAGE_JPEG),y)
LIBGDIPLUS_CONF_OPTS += --with-libjpeg=$(STAGING_DIR)/usr
LIBGDIPLUS_DEPENDENCIES += jpeg
else
LIBGDIPLUS_CONF_OPTS += --without-libjpeg
endif

ifeq ($(LINGMO_PACKAGE_TIFF),y)
LIBGDIPLUS_CONF_OPTS += --with-libtiff=$(STAGING_DIR)/usr
LIBGDIPLUS_DEPENDENCIES += tiff
else
LIBGDIPLUS_CONF_OPTS += --without-libtiff
endif

$(eval $(autotools-package))
