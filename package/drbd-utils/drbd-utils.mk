################################################################################
#
# drbd-utils
#
################################################################################

DRBD_UTILS_VERSION = 9.21.4
DRBD_UTILS_SITE = http://pkg.linbit.com/downloads/drbd/utils
DRBD_UTILS_LICENSE = GPL-2.0+
DRBD_UTILS_LICENSE_FILES = COPYING
DRBD_UTILS_SELINUX_MODULES = drbd
DRBD_UTILS_DEPENDENCIES = host-flex

DRBD_UTILS_CONF_OPTS = --with-distro=generic --without-manual

ifeq ($(LINGMO_INIT_SYSTEMD),y)
DRBD_UTILS_CONF_OPTS += --with-initscripttype=systemd
DRBD_UTILS_DEPENDENCIES += systemd
else
DRBD_UTILS_CONF_OPTS += --with-initscripttype=sysv
endif

ifeq ($(LINGMO_TOOLCHAIN_HAS_THREADS),y)
DRBD_UTILS_CONF_OPTS += --with-drbdmon
else
DRBD_UTILS_CONF_OPTS += --without-drbdmon
endif

ifeq ($(LINGMO_PACKAGE_HAS_UDEV),y)
DRBD_UTILS_CONF_OPTS += --with-udev=yes
DRBD_UTILS_DEPENDENCIES += udev
else
DRBD_UTILS_CONF_OPTS += --with-udev=no
endif

$(eval $(autotools-package))
