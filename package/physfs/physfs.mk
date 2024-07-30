################################################################################
#
# physfs
#
################################################################################

PHYSFS_VERSION = 3.0.2
PHYSFS_SOURCE = physfs-$(PHYSFS_VERSION).tar.bz2
PHYSFS_SITE = https://icculus.org/physfs/downloads

PHYSFS_LICENSE = Zlib
PHYSFS_LICENSE_FILES = LICENSE.txt

PHYSFS_INSTALL_STAGING = YES

PHYSFS_CONF_OPTS = -DPHYSFS_BUILD_TEST=OFF

ifeq ($(LINGMO_SHARED_LIBS)$(LINGMO_SHARED_STATIC_LIBS),y)
PHYSFS_CONF_OPTS += -DPHYSFS_BUILD_SHARED=ON
else
PHYSFS_CONF_OPTS += -DPHYSFS_BUILD_SHARED=OFF
endif

ifeq ($(LINGMO_STATIC_LIBS)$(LINGMO_SHARED_STATIC_LIBS),y)
PHYSFS_CONF_OPTS += -DPHYSFS_BUILD_STATIC=ON
else
PHYSFS_CONF_OPTS += -DPHYSFS_BUILD_STATIC=OFF
endif

$(eval $(cmake-package))
