################################################################################
#
# Build the ext2 root filesystem image
#
################################################################################

ROOTFS_EXT2_SIZE = $(call qstrip,$(LINGMO_TARGET_ROOTFS_EXT2_SIZE))
ifeq ($(LINGMO_TARGET_ROOTFS_EXT2)-$(ROOTFS_EXT2_SIZE),y-)
$(error LINGMO_TARGET_ROOTFS_EXT2_SIZE cannot be empty)
endif

ROOTFS_EXT2_MKFS_OPTS = $(call qstrip,$(LINGMO_TARGET_ROOTFS_EXT2_MKFS_OPTIONS))

# qstrip results in stripping consecutive spaces into a single one. So the
# variable is not qstrip-ed to preserve the integrity of the string value.
ROOTFS_EXT2_LABEL = $(subst ",,$(LINGMO_TARGET_ROOTFS_EXT2_LABEL))
#" Syntax highlighting... :-/ )

ROOTFS_EXT2_OPTS = \
	-d $(TARGET_DIR) \
	-r $(LINGMO_TARGET_ROOTFS_EXT2_REV) \
	-N $(LINGMO_TARGET_ROOTFS_EXT2_INODES) \
	-m $(LINGMO_TARGET_ROOTFS_EXT2_RESBLKS) \
	-L "$(ROOTFS_EXT2_LABEL)" \
	-I $(LINGMO_TARGET_ROOTFS_EXT2_INODE_SIZE) \
	$(ROOTFS_EXT2_MKFS_OPTS)

ROOTFS_EXT2_DEPENDENCIES = host-e2fsprogs

define ROOTFS_EXT2_CMD
	rm -f $@
	$(HOST_DIR)/sbin/mkfs.ext$(LINGMO_TARGET_ROOTFS_EXT2_GEN) $(ROOTFS_EXT2_OPTS) $@ \
		"$(ROOTFS_EXT2_SIZE)" \
	|| { ret=$$?; \
	     echo "*** Maybe you need to increase the filesystem size (LINGMO_TARGET_ROOTFS_EXT2_SIZE)" 1>&2; \
	     exit $$ret; \
	}
endef

ifneq ($(LINGMO_TARGET_ROOTFS_EXT2_GEN),2)
define ROOTFS_EXT2_SYMLINK
	ln -sf rootfs.ext2$(ROOTFS_EXT2_COMPRESS_EXT) $(BINARIES_DIR)/rootfs.ext$(LINGMO_TARGET_ROOTFS_EXT2_GEN)$(ROOTFS_EXT2_COMPRESS_EXT)
endef
ROOTFS_EXT2_POST_GEN_HOOKS += ROOTFS_EXT2_SYMLINK
endif

$(eval $(rootfs))
