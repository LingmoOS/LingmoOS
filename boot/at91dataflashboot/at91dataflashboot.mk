################################################################################
#
# at91dataflashboot
#
################################################################################

AT91DATAFLASHBOOT_VERSION = 1.05
AT91DATAFLASHBOOT_SOURCE = DataflashBoot-$(AT91DATAFLASHBOOT_VERSION).tar.bz2
AT91DATAFLASHBOOT_SITE = ftp://www.at91.com/pub/buildroot

AT91DATAFLASHBOOT_INSTALL_TARGET = NO
AT91DATAFLASHBOOT_INSTALL_IMAGES = YES

AT91DATAFLASHBOOT_CFLAGS = $(TARGET_CFLAGS) -fno-stack-protector
ifeq ($(LINGMO_ARM_INSTRUCTIONS_THUMB),y)
AT91DATAFLASHBOOT_CFLAGS += -marm
endif

define AT91DATAFLASHBOOT_BUILD_CMDS
	make -C $(@D) CROSS_COMPILE=$(TARGET_CROSS) \
		CFLAGS="$(AT91DATAFLASHBOOT_CFLAGS)"
endef

define AT91DATAFLASHBOOT_INSTALL_IMAGES_CMDS
	cp $(@D)/DataflashBoot-$(AT91DATAFLASHBOOT_VERSION).bin $(BINARIES_DIR)
endef

$(eval $(generic-package))
