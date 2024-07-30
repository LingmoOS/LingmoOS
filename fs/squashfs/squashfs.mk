################################################################################
#
# Build the squashfs root filesystem image
#
################################################################################

ROOTFS_SQUASHFS_DEPENDENCIES = host-squashfs

ROOTFS_SQUASHFS_ARGS = \
	-noappend \
	-processors $(PARALLEL_JOBS) \
	-b $(call qstrip,$(LINGMO_TARGET_ROOTFS_SQUASHFS_BS)) \
	$(call qstrip,$(LINGMO_TARGET_ROOTFS_SQUASHFS_COMP_OPTS))

ifeq ($(LINGMO_TARGET_ROOTFS_SQUASHFS_PAD),)
ROOTFS_SQUASHFS_ARGS += -nopad
endif

ifeq ($(LINGMO_TARGET_ROOTFS_SQUASHFS4_LZ4),y)
ROOTFS_SQUASHFS_ARGS += -comp lz4
else ifeq ($(LINGMO_TARGET_ROOTFS_SQUASHFS4_LZO),y)
ROOTFS_SQUASHFS_ARGS += -comp lzo
else ifeq ($(LINGMO_TARGET_ROOTFS_SQUASHFS4_LZMA),y)
ROOTFS_SQUASHFS_ARGS += -comp lzma
else ifeq ($(LINGMO_TARGET_ROOTFS_SQUASHFS4_XZ),y)
ROOTFS_SQUASHFS_ARGS += -comp xz
else ifeq ($(LINGMO_TARGET_ROOTFS_SQUASHFS4_ZSTD),y)
ROOTFS_SQUASHFS_ARGS += -comp zstd
else
ROOTFS_SQUASHFS_ARGS += -comp gzip
endif

define ROOTFS_SQUASHFS_CMD
	$(HOST_DIR)/bin/mksquashfs $(TARGET_DIR) $@ $(ROOTFS_SQUASHFS_ARGS)
endef

$(eval $(rootfs))
