################################################################################
#
# Build the EROFS root filesystem image
#
################################################################################

ROOTFS_EROFS_DEPENDENCIES = host-erofs-utils

ifeq ($(LINGMO_TARGET_ROOTFS_EROFS_LZ4HC),y)
ROOTFS_EROFS_ARGS += -zlz4hc
endif

ifeq ($(LINGMO_REPRODUCIBLE),y)
ROOTFS_EROFS_ARGS += \
	-T $(SOURCE_DATE_EPOCH) \
	-U 00000000-0000-0000-0000-000000000000
endif

ifneq ($(LINGMO_TARGET_ROOTFS_EROFS_PCLUSTERSIZE),0)
ROOTFS_EROFS_ARGS += -C$(strip $(LINGMO_TARGET_ROOTFS_EROFS_PCLUSTERSIZE))
endif

define ROOTFS_EROFS_CMD
	$(HOST_DIR)/bin/mkfs.erofs $(ROOTFS_EROFS_ARGS) $@ $(TARGET_DIR)
endef

$(eval $(rootfs))
