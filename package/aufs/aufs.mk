################################################################################
#
# aufs
#
################################################################################

AUFS_VERSION = $(call qstrip,$(LINGMO_PACKAGE_AUFS_VERSION))
AUFS_LICENSE = GPL-2.0
AUFS_LICENSE_FILES = COPYING

ifeq ($(LINGMO_PACKAGE_AUFS_SERIES),3)
AUFS_SITE = https://git.code.sf.net/p/aufs/aufs3-standalone
AUFS_SITE_METHOD = git
else ifeq ($(LINGMO_PACKAGE_AUFS_SERIES),4)
AUFS_SITE = $(call github,sfjro,aufs4-standalone,$(AUFS_VERSION))
else ifeq ($(LINGMO_PACKAGE_AUFS_SERIES),5)
AUFS_SITE = $(call github,sfjro,aufs5-standalone,$(AUFS_VERSION))
endif

ifeq ($(BR_BUILDING):$(LINGMO_PACKAGE_AUFS):$(AUFS_VERSION),y:y:)
$(error No aufs version specified)
endif

$(eval $(generic-package))
