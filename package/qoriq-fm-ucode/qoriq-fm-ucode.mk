################################################################################
#
# qoriq-fm-ucode
#
################################################################################

QORIQ_FM_UCODE_VERSION = LSDK-21.08
QORIQ_FM_UCODE_SITE = $(call github,nxp-qoriq,qoriq-fm-ucode,$(QORIQ_FM_UCODE_VERSION))
QORIQ_FM_UCODE_LICENSE = NXP Binary EULA
QORIQ_FM_UCODE_LICENSE_FILES = NXP-Binary-EULA.txt
QORIQ_FM_UCODE_INSTALL_IMAGES = YES
QORIQ_FM_UCODE_INSTALL_TARGET = NO

QORIQ_FM_UCODE_PLATFORM = $(call qstrip,$(LINGMO_PACKAGE_QORIQ_FM_UCODE_PLATFORM))

define QORIQ_FM_UCODE_INSTALL_IMAGES_CMDS
	cp $(@D)/fsl_fman_ucode_$(QORIQ_FM_UCODE_PLATFORM)*.bin \
		$(BINARIES_DIR)
endef

$(eval $(generic-package))
