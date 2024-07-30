################################################################################
#
# libvips
#
################################################################################

LIBVIPS_VERSION = 8.10.6
LIBVIPS_SOURCE = vips-$(LIBVIPS_VERSION).tar.gz
LIBVIPS_SITE = https://github.com/libvips/libvips/releases/download/v$(LIBVIPS_VERSION)
LIBVIPS_LICENSE = LGPL-2.1+
LIBVIPS_LICENSE_FILES = COPYING
LIBVIPS_CPE_ID_VENDOR = libvips

# Sparc64 compile fails, for all optimization levels except -O0. To
# fix the problem, use -O0 with no optimization instead. Bug reported
# upstream at https://gcc.gnu.org/bugzilla/show_bug.cgi?id=69038.
ifeq ($(LINGMO_sparc64),y)
LIBVIPS_CXXFLAGS += -O0
endif

LIBVIPS_CONF_ENV += CXXFLAGS="$(TARGET_CXXFLAGS) $(LIBVIPS_CXXFLAGS)" \
	LIBS=$(TARGET_NLS_LIBS)

LIBVIPS_CONF_OPTS = \
	--without-dmalloc \
	--without-gsf \
	--without-OpenEXR \
	--without-openslide \
	--without-cfitsio \
	--without-pangoft2 \
	--without-x
LIBVIPS_INSTALL_STAGING = YES
LIBVIPS_DEPENDENCIES = \
	host-pkgconf expat libglib2 \
	$(TARGET_NLS_DEPENDENCIES)

ifeq ($(LINGMO_PACKAGE_GIFLIB),y)
LIBVIPS_CONF_OPTS += --with-giflib
LIBVIPS_DEPENDENCIES += giflib
else
LIBVIPS_CONF_OPTS += --without-giflib
endif

ifeq ($(LINGMO_PACKAGE_GOBJECT_INTROSPECTION),y)
LIBVIPS_CONF_OPTS += --enable-introspection
LIBVIPS_DEPENDENCIES += gobject-introspection
else
LIBVIPS_CONF_OPTS += --disable-introspection
endif

ifeq ($(LINGMO_PACKAGE_IMAGEMAGICK),y)
LIBVIPS_CONF_OPTS += \
	--with-magick \
	--with-magickpackage=MagickCore
LIBVIPS_DEPENDENCIES += imagemagick
else ifeq ($(LINGMO_PACKAGE_GRAPHICSMAGICK),y)
LIBVIPS_CONF_OPTS += \
	--with-magick \
	--with-magickpackage=GraphicsMagick
LIBVIPS_DEPENDENCIES += graphicsmagick
else
LIBVIPS_CONF_OPTS += --without-magick
endif

ifeq ($(LINGMO_PACKAGE_JPEG),y)
LIBVIPS_CONF_OPTS += --with-jpeg
LIBVIPS_DEPENDENCIES += jpeg
else
LIBVIPS_CONF_OPTS += --without-jpeg
endif

ifeq ($(LINGMO_PACKAGE_LCMS2),y)
LIBVIPS_CONF_OPTS += --with-lcms
LIBVIPS_DEPENDENCIES += lcms2
else
LIBVIPS_CONF_OPTS += --without-lcms
endif

ifeq ($(LINGMO_PACKAGE_LIBPNG),y)
LIBVIPS_CONF_OPTS += --with-png
LIBVIPS_DEPENDENCIES += libpng
else
LIBVIPS_CONF_OPTS += --without-png
endif

ifeq ($(LINGMO_PACKAGE_LIBRSVG),y)
LIBVIPS_CONF_OPTS += --with-rsvg
LIBVIPS_DEPENDENCIES += librsvg
else
LIBVIPS_CONF_OPTS += --without-rsvg
endif

ifeq ($(LINGMO_PACKAGE_MATIO),y)
LIBVIPS_CONF_OPTS += --with-matio
LIBVIPS_DEPENDENCIES += matio
else
LIBVIPS_CONF_OPTS += --without-matio
endif

ifeq ($(LINGMO_PACKAGE_ORC),y)
LIBVIPS_CONF_OPTS += --with-orc
LIBVIPS_DEPENDENCIES += orc
else
LIBVIPS_CONF_OPTS += --without-orc
endif

ifeq ($(LINGMO_PACKAGE_POPPLER),y)
LIBVIPS_CONF_OPTS += --with-poppler
LIBVIPS_DEPENDENCIES += poppler
else
LIBVIPS_CONF_OPTS += --without-poppler
endif

ifeq ($(LINGMO_PACKAGE_TIFF),y)
LIBVIPS_CONF_OPTS += --with-tiff
LIBVIPS_DEPENDENCIES += tiff
else
LIBVIPS_CONF_OPTS += --without-tiff
endif

ifeq ($(LINGMO_PACKAGE_FFTW_DOUBLE),y)
LIBVIPS_CONF_OPTS += --with-fftw
LIBVIPS_DEPENDENCIES += fftw-double
else
LIBVIPS_CONF_OPTS += --without-fftw
endif

ifeq ($(LINGMO_PACKAGE_LIBEXIF),y)
LIBVIPS_CONF_OPTS += --with-libexif
LIBVIPS_DEPENDENCIES += libexif
else
LIBVIPS_CONF_OPTS += --without-libexif
endif

ifeq ($(LINGMO_PACKAGE_LIBHEIF),y)
LIBVIPS_CONF_OPTS += --with-heif
LIBVIPS_DEPENDENCIES += libheif
else
LIBVIPS_CONF_OPTS += --without-heif
endif

ifeq ($(LINGMO_PACKAGE_WEBP_DEMUX)$(LINGMO_PACKAGE_WEBP_MUX),yy)
LIBVIPS_CONF_OPTS += --with-libwebp
LIBVIPS_DEPENDENCIES += webp
else
LIBVIPS_CONF_OPTS += --without-libwebp
endif

ifeq ($(LINGMO_PACKAGE_ZLIB),y)
LIBVIPS_CONF_OPTS += --with-zlib
LIBVIPS_DEPENDENCIES += zlib
else
LIBVIPS_CONF_OPTS += --without-zlib
endif

$(eval $(autotools-package))
