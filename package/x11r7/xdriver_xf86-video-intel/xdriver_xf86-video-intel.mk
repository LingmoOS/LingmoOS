################################################################################
#
# xdriver_xf86-video-intel
#
################################################################################

XDRIVER_XF86_VIDEO_INTEL_VERSION = 31486f40f8e8f8923ca0799aea84b58799754564
XDRIVER_XF86_VIDEO_INTEL_SITE = git://anongit.freedesktop.org/xorg/driver/xf86-video-intel
XDRIVER_XF86_VIDEO_INTEL_LICENSE = MIT
XDRIVER_XF86_VIDEO_INTEL_LICENSE_FILES = COPYING
XDRIVER_XF86_VIDEO_INTEL_AUTORECONF = YES

# -D_GNU_SOURCE fixes a getline-related compile error in src/sna/kgem.c
# We force -O2 regardless of the optimization level chosen by the user,
# as compiling this package is known to be broken with -Os.
XDRIVER_XF86_VIDEO_INTEL_CONF_ENV = \
	CFLAGS="$(TARGET_CFLAGS) -D_GNU_SOURCE -O2"

XDRIVER_XF86_VIDEO_INTEL_CONF_OPTS = \
	--disable-xvmc \
	--enable-sna \
	--disable-xaa \
	--disable-dga \
	--disable-tools \
	--disable-async-swap

XDRIVER_XF86_VIDEO_INTEL_DEPENDENCIES = \
	libdrm \
	libpciaccess \
	xlib_libXrandr \
	xorgproto \
	xserver_xorg-server

# DRI support is provided by xserver_xorg-server if libgl is enabled
ifeq ($(LINGMO_PACKAGE_HAS_LIBGL),y)
XDRIVER_XF86_VIDEO_INTEL_CONF_OPTS += \
	--enable-dri2 \
	--enable-dri3 \
	--enable-uxa
else
XDRIVER_XF86_VIDEO_INTEL_CONF_OPTS += \
	--disable-dri2 \
	--disable-dri3 \
	--disable-uxa
endif

$(eval $(autotools-package))
