################################################################################
#
# cairo
#
################################################################################

CAIRO_VERSION = 1.17.4
CAIRO_SOURCE = cairo-$(CAIRO_VERSION).tar.xz
CAIRO_LICENSE = LGPL-2.1 or MPL-1.1 (library)
CAIRO_LICENSE_FILES = COPYING COPYING-LGPL-2.1 COPYING-MPL-1.1
CAIRO_CPE_ID_VENDOR = cairographics
CAIRO_SITE = http://cairographics.org/snapshots
CAIRO_INSTALL_STAGING = YES

# 0001-_arc_max_angle_for_tolerance_normalized-fix-infinite.patch
CAIRO_IGNORE_CVES += CVE-2019-6462
# 0002-Fix-mask-usage-in-image-compositor.patch
CAIRO_IGNORE_CVES += CVE-2020-35492

CAIRO_CONF_ENV = LIBS="$(CAIRO_LIBS)"

# relocation truncated to fit: R_68K_GOT16O
ifeq ($(LINGMO_m68k_cf),y)
CAIRO_CONF_ENV += CFLAGS="$(TARGET_CFLAGS) -mxgot"
endif

ifeq ($(LINGMO_TOOLCHAIN_HAS_THREADS_NPTL),)
CAIRO_CONF_ENV += CPPFLAGS="$(TARGET_CPPFLAGS) -DCAIRO_NO_MUTEX=1"
endif

# cairo can use C++11 atomics when available, so we need to link with
# libatomic for the architectures who need libatomic.
ifeq ($(LINGMO_TOOLCHAIN_HAS_LIBATOMIC),y)
CAIRO_LIBS += -latomic
endif

CAIRO_CONF_OPTS = \
	--enable-trace=no \
	--enable-interpreter=no

CAIRO_DEPENDENCIES = host-pkgconf fontconfig pixman

# Just the bare minimum to make other host-* packages happy
HOST_CAIRO_CONF_OPTS = \
	--enable-trace=no \
	--enable-interpreter=no \
	--disable-directfb \
	--enable-ft \
	--enable-gobject \
	--disable-glesv2 \
	--disable-vg \
	--disable-xlib \
	--disable-xcb \
	--without-x \
	--disable-xlib-xrender \
	--disable-ps \
	--disable-pdf \
	--enable-png \
	--enable-script \
	--disable-svg \
	--disable-tee \
	--disable-xml
HOST_CAIRO_DEPENDENCIES = \
	host-freetype \
	host-fontconfig \
	host-libglib2 \
	host-libpng \
	host-pixman \
	host-pkgconf

# DirectFB svg support rely on Cairo and Cairo DirectFB support depends on
# DirectFB. Break circular dependency by disabling DirectFB support in Cairo
# (which is experimental)
ifeq ($(LINGMO_PACKAGE_DIRECTFB)x$(LINGMO_PACKAGE_DIRECTFB_SVG),yx)
CAIRO_CONF_OPTS += --enable-directfb
CAIRO_DEPENDENCIES += directfb
else
CAIRO_CONF_OPTS += --disable-directfb
endif

ifeq ($(LINGMO_PACKAGE_FREETYPE),y)
CAIRO_CONF_OPTS += --enable-ft
CAIRO_DEPENDENCIES += freetype
else
CAIRO_CONF_OPTS += --disable-ft
endif

ifeq ($(LINGMO_PACKAGE_LIBEXECINFO),y)
CAIRO_DEPENDENCIES += libexecinfo
CAIRO_LIBS += -lexecinfo
endif

ifeq ($(LINGMO_PACKAGE_LIBGLIB2),y)
CAIRO_CONF_OPTS += --enable-gobject
CAIRO_DEPENDENCIES += libglib2
else
CAIRO_CONF_OPTS += --disable-gobject
endif

# Can use GL or GLESv2 but not both
ifeq ($(LINGMO_PACKAGE_HAS_LIBGL),y)
CAIRO_CONF_OPTS += --enable-gl --disable-glesv2
CAIRO_DEPENDENCIES += libgl
else
ifeq ($(LINGMO_PACKAGE_HAS_LIBGLES),y)
CAIRO_CONF_OPTS += --disable-gl --enable-glesv2
CAIRO_DEPENDENCIES += libgles
else
CAIRO_CONF_OPTS += --disable-gl --disable-glesv2
endif
endif

ifeq ($(LINGMO_PACKAGE_HAS_LIBOPENVG),y)
CAIRO_CONF_OPTS += --enable-vg
CAIRO_DEPENDENCIES += libopenvg
else
CAIRO_CONF_OPTS += --disable-vg
endif

ifeq ($(LINGMO_PACKAGE_LZO),y)
CAIRO_DEPENDENCIES += lzo
endif

ifeq ($(LINGMO_PACKAGE_XORG7),y)
CAIRO_CONF_OPTS += --enable-xlib --enable-xcb --with-x
CAIRO_DEPENDENCIES += xlib_libX11 xlib_libXext
else
CAIRO_CONF_OPTS += --disable-xlib --disable-xcb --without-x
endif

ifeq ($(LINGMO_PACKAGE_XLIB_LIBXRENDER),y)
CAIRO_CONF_OPTS += --enable-xlib-xrender
CAIRO_DEPENDENCIES += xlib_libXrender
else
CAIRO_CONF_OPTS += --disable-xlib-xrender
endif

ifeq ($(LINGMO_PACKAGE_CAIRO_PS),y)
CAIRO_CONF_OPTS += --enable-ps
CAIRO_DEPENDENCIES += zlib
else
CAIRO_CONF_OPTS += --disable-ps
endif

ifeq ($(LINGMO_PACKAGE_CAIRO_PDF),y)
CAIRO_CONF_OPTS += --enable-pdf
CAIRO_DEPENDENCIES += zlib
else
CAIRO_CONF_OPTS += --disable-pdf
endif

ifeq ($(LINGMO_PACKAGE_CAIRO_PNG),y)
CAIRO_CONF_OPTS += --enable-png
CAIRO_DEPENDENCIES += libpng
else
CAIRO_CONF_OPTS += --disable-png
endif

ifeq ($(LINGMO_PACKAGE_CAIRO_SCRIPT),y)
CAIRO_CONF_OPTS += --enable-script
else
CAIRO_CONF_OPTS += --disable-script
endif

ifeq ($(LINGMO_PACKAGE_CAIRO_SVG),y)
CAIRO_CONF_OPTS += --enable-svg
else
CAIRO_CONF_OPTS += --disable-svg
endif

ifeq ($(LINGMO_PACKAGE_CAIRO_TEE),y)
CAIRO_CONF_OPTS += --enable-tee
else
CAIRO_CONF_OPTS += --disable-tee
endif

ifeq ($(LINGMO_PACKAGE_CAIRO_XML),y)
CAIRO_CONF_OPTS += --enable-xml
else
CAIRO_CONF_OPTS += --disable-xml
endif

$(eval $(autotools-package))
$(eval $(host-autotools-package))
