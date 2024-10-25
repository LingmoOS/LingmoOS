################################################################################
#
# xdriver_xf86-video-tdfx
#
################################################################################

XDRIVER_XF86_VIDEO_TDFX_VERSION = 1.5.0
XDRIVER_XF86_VIDEO_TDFX_SITE = http://xorg.freedesktop.org/releases/individual/driver
XDRIVER_XF86_VIDEO_TDFX_SOURCE = xf86-video-tdfx-$(XDRIVER_XF86_VIDEO_TDFX_VERSION).tar.bz2
XDRIVER_XF86_VIDEO_TDFX_LICENSE = MIT
XDRIVER_XF86_VIDEO_TDFX_LICENSE_FILES = COPYING
XDRIVER_XF86_VIDEO_TDFX_AUTORECONF = YES
XDRIVER_XF86_VIDEO_TDFX_DEPENDENCIES = \
	libdrm \
	xorgproto \
	xserver_xorg-server

ifeq ($(LINGMO_PACKAGE_HAS_LIBGL),y)
XDRIVER_XF86_VIDEO_TDFX_CONF_OPTS += --enable-dri
else
XDRIVER_XF86_VIDEO_TDFX_CONF_OPTS += --disable-dri
endif

$(eval $(autotools-package))
