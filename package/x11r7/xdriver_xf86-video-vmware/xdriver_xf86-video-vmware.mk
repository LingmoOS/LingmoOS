################################################################################
#
# xdriver_xf86-video-vmware
#
################################################################################

XDRIVER_XF86_VIDEO_VMWARE_VERSION = 13.4.0
XDRIVER_XF86_VIDEO_VMWARE_SOURCE = xf86-video-vmware-$(XDRIVER_XF86_VIDEO_VMWARE_VERSION).tar.xz
XDRIVER_XF86_VIDEO_VMWARE_SITE = https://xorg.freedesktop.org/archive/individual/driver
XDRIVER_XF86_VIDEO_VMWARE_LICENSE = MIT
XDRIVER_XF86_VIDEO_VMWARE_LICENSE_FILES = COPYING
XDRIVER_XF86_VIDEO_VMWARE_DEPENDENCIES = mesa3d xserver_xorg-server xorgproto

ifeq ($(LINGMO_PACKAGE_HAS_UDEV),y)
XDRIVER_XF86_VIDEO_VMWARE_CONF_OPTS += --with-libudev
XDRIVER_XF86_VIDEO_VMWARE_DEPENDENCIES += udev
else
XDRIVER_XF86_VIDEO_VMWARE_CONF_OPTS += --without-libudev
endif

$(eval $(autotools-package))
