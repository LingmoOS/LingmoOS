################################################################################
#
# Build the cramfs root filesystem image
#
################################################################################

ifeq ($(LINGMO_ENDIAN),"BIG")
CRAMFS_OPTS = -B
else
CRAMFS_OPTS = -L
endif

ifeq ($(LINGMO_TARGET_ROOTFS_CRAMFS_XIP),y)
ifeq ($(LINGMO_USE_MMU),y)
CRAMFS_OPTS += -X -X
else
CRAMFS_OPTS += -X
endif
endif

define ROOTFS_CRAMFS_CMD
	$(HOST_DIR)/bin/mkcramfs $(CRAMFS_OPTS) $(TARGET_DIR) $@
endef

ROOTFS_CRAMFS_DEPENDENCIES = host-cramfs

$(eval $(rootfs))
