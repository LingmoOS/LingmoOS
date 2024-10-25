################################################################################
#
# Build the ubifs root filesystem image
#
################################################################################

UBIFS_OPTS = \
	-e $(LINGMO_TARGET_ROOTFS_UBIFS_LEBSIZE) \
	-c $(LINGMO_TARGET_ROOTFS_UBIFS_MAXLEBCNT) \
	-m $(LINGMO_TARGET_ROOTFS_UBIFS_MINIOSIZE)

ifeq ($(LINGMO_TARGET_ROOTFS_UBIFS_RT_ZLIB),y)
UBIFS_OPTS += -x zlib
endif
ifeq ($(LINGMO_TARGET_ROOTFS_UBIFS_RT_LZO),y)
UBIFS_OPTS += -x lzo
endif
ifeq ($(LINGMO_TARGET_ROOTFS_UBIFS_RT_NONE),y)
UBIFS_OPTS += -x none
endif

UBIFS_OPTS += $(call qstrip,$(LINGMO_TARGET_ROOTFS_UBIFS_OPTS))

ROOTFS_UBIFS_DEPENDENCIES = host-mtd

define ROOTFS_UBIFS_CMD
	$(HOST_DIR)/sbin/mkfs.ubifs -d $(TARGET_DIR) $(UBIFS_OPTS) -o $@
endef

$(eval $(rootfs))
