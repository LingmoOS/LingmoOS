################################################################################
#
# Embed the ubifs image into an ubi image
#
################################################################################

UBI_UBINIZE_OPTS = -m $(LINGMO_TARGET_ROOTFS_UBIFS_MINIOSIZE)
UBI_UBINIZE_OPTS += -p $(LINGMO_TARGET_ROOTFS_UBI_PEBSIZE)
ifneq ($(LINGMO_TARGET_ROOTFS_UBI_SUBSIZE),0)
UBI_UBINIZE_OPTS += -s $(LINGMO_TARGET_ROOTFS_UBI_SUBSIZE)
endif

UBI_UBINIZE_OPTS += $(call qstrip,$(LINGMO_TARGET_ROOTFS_UBI_OPTS))

ROOTFS_UBI_DEPENDENCIES = rootfs-ubifs

ifeq ($(LINGMO_TARGET_ROOTFS_UBI_USE_CUSTOM_CONFIG),y)
UBI_UBINIZE_CONFIG_FILE_PATH = $(call qstrip,$(LINGMO_TARGET_ROOTFS_UBI_CUSTOM_CONFIG_FILE))
else
UBI_UBINIZE_CONFIG_FILE_PATH = fs/ubi/ubinize.cfg
endif

# don't use sed -i as it misbehaves on systems with SELinux enabled when this is
# executed through fakeroot (see #9386)
define ROOTFS_UBI_CMD
	sed 's;LINGMO_ROOTFS_UBIFS_PATH;$@fs;;s;BINARIES_DIR;$(BINARIES_DIR);' \
		$(UBI_UBINIZE_CONFIG_FILE_PATH) > $(BUILD_DIR)/ubinize.cfg
	$(HOST_DIR)/sbin/ubinize -o $@ $(UBI_UBINIZE_OPTS) $(BUILD_DIR)/ubinize.cfg
	rm $(BUILD_DIR)/ubinize.cfg
endef

$(eval $(rootfs))
