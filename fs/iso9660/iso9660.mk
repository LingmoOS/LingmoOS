################################################################################
#
# Build the iso96600 root filesystem image
#
################################################################################

#
# We need to handle three cases:
#
#  1. The ISO9660 filesystem will really be the real root filesystem
#     itself. This is when LINGMO_TARGET_ROOTFS_ISO9660_INITRD is
#     disabled.
#
#  2. The ISO9660 filesystem will be a filesystem with just a kernel
#     image, initrd and grub. This is when
#     LINGMO_TARGET_ROOTFS_ISO9660_INITRD is enabled, but
#     LINGMO_TARGET_ROOTFS_INITRAMFS is disabled.
#
#  3. The ISO9660 filesystem will be a filesystem with just a kernel
#     image and grub. This is like (2), except that the initrd is
#     built into the kernel image. This is when
#     LINGMO_TARGET_ROOTFS_INITRAMFS is enabled (regardless of the value
#     of LINGMO_TARGET_ROOTFS_ISO9660_INITRD).

ROOTFS_ISO9660_BOOT_MENU = $(call qstrip,$(LINGMO_TARGET_ROOTFS_ISO9660_BOOT_MENU))

ROOTFS_ISO9660_DEPENDENCIES = host-xorriso linux

ifeq ($(LINGMO_TARGET_ROOTFS_INITRAMFS),y)
ROOTFS_ISO9660_USE_INITRD = YES
endif

ifeq ($(LINGMO_TARGET_ROOTFS_ISO9660_INITRD),y)
ROOTFS_ISO9660_USE_INITRD = YES
endif

ifeq ($(ROOTFS_ISO9660_USE_INITRD),YES)
ROOTFS_ISO9660_TMP_TARGET_DIR = $(FS_DIR)/rootfs.iso9660.tmp
define ROOTFS_ISO9660_CREATE_TEMPDIR
	$(RM) -rf $(ROOTFS_ISO9660_TMP_TARGET_DIR)
	mkdir -p $(ROOTFS_ISO9660_TMP_TARGET_DIR)
endef
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_CREATE_TEMPDIR
else ifeq ($(LINGMO_TARGET_ROOTFS_ISO9660_TRANSPARENT_COMPRESSION),y)
ROOTFS_ISO9660_DEPENDENCIES += host-zisofs-tools
ROOTFS_ISO9660_TMP_TARGET_DIR = $(FS_DIR)/rootfs.iso9660.tmp
# This must be early, before we copy the bootloader files.
define ROOTFS_ISO9660_MKZFTREE
	$(RM) -rf $(ROOTFS_ISO9660_TMP_TARGET_DIR)
	$(HOST_DIR)/bin/mkzftree -X -z 9 -p $(PARALLEL_JOBS) \
		$(TARGET_DIR) \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)
endef
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_MKZFTREE
ROOTFS_ISO9660_OPTS += -z
else
ROOTFS_ISO9660_TMP_TARGET_DIR = $(TARGET_DIR)
endif

ifeq ($(LINGMO_REPRODUCIBLE),y)
ROOTFS_ISO9660_VFAT_OPTS = --invariant
ROOTFS_ISO9660_FIX_TIME = touch -d @$(SOURCE_DATE_EPOCH)
else
ROOTFS_ISO9660_FIX_TIME = :
endif

ifeq ($(LINGMO_TARGET_ROOTFS_ISO9660_GRUB2)$(LINGMO_TARGET_ROOTFS_ISO9660_BIOS_BOOTLOADER),yy)
ROOTFS_ISO9660_DEPENDENCIES += grub2
ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH = \
	$(ROOTFS_ISO9660_TMP_TARGET_DIR)/boot/grub/grub.cfg
ROOTFS_ISO9660_BOOT_IMAGE = boot/grub/grub-eltorito.img
define ROOTFS_ISO9660_INSTALL_BOOTLOADER_BIOS
	$(INSTALL) -D -m 0644 $(BINARIES_DIR)/grub-eltorito.img \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)/boot/grub/grub-eltorito.img
endef
endif

ifeq ($(LINGMO_TARGET_ROOTFS_ISO9660_GRUB2)$(LINGMO_TARGET_ROOTFS_ISO9660_EFI_BOOTLOADER),yy)
ROOTFS_ISO9660_DEPENDENCIES += grub2 host-dosfstools host-mtools
ROOTFS_ISO9660_EFI_PARTITION = boot/fat.efi
ROOTFS_ISO9660_EFI_PARTITION_PATH = $(ROOTFS_ISO9660_TMP_TARGET_DIR)/$(ROOTFS_ISO9660_EFI_PARTITION)
ROOTFS_ISO9660_EFI_PARTITION_CONTENT = $(BINARIES_DIR)/efi-part
ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH = \
	$(ROOTFS_ISO9660_TMP_TARGET_DIR)/boot/grub/grub.cfg
define ROOTFS_ISO9660_INSTALL_BOOTLOADER_EFI
	rm -rf $(ROOTFS_ISO9660_EFI_PARTITION_PATH)
	mkdir -p $(dir $(ROOTFS_ISO9660_EFI_PARTITION_PATH))
	dd if=/dev/zero of=$(ROOTFS_ISO9660_EFI_PARTITION_PATH) bs=1M count=1
	$(HOST_DIR)/sbin/mkfs.vfat $(ROOTFS_ISO9660_VFAT_OPTS) $(ROOTFS_ISO9660_EFI_PARTITION_PATH)
	$(ROOTFS_ISO9660_FIX_TIME) $(ROOTFS_ISO9660_EFI_PARTITION_CONTENT)/*
	$(HOST_DIR)/bin/mcopy -p -m -i $(ROOTFS_ISO9660_EFI_PARTITION_PATH) -s \
		$(ROOTFS_ISO9660_EFI_PARTITION_CONTENT)/* ::/
	$(ROOTFS_ISO9660_FIX_TIME) $(ROOTFS_ISO9660_EFI_PARTITION_PATH)
endef
endif

ifeq ($(LINGMO_TARGET_ROOTFS_ISO9660_ISOLINUX),y)
ROOTFS_ISO9660_DEPENDENCIES += syslinux
ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH = \
	$(ROOTFS_ISO9660_TMP_TARGET_DIR)/isolinux/isolinux.cfg
ROOTFS_ISO9660_BOOT_IMAGE = isolinux/isolinux.bin
define ROOTFS_ISO9660_INSTALL_BOOTLOADER_BIOS
	$(INSTALL) -D -m 0644 $(BINARIES_DIR)/syslinux/* \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)/isolinux/
	$(INSTALL) -D -m 0644 $(HOST_DIR)/share/syslinux/ldlinux.c32 \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)/isolinux/ldlinux.c32
endef
endif

define ROOTFS_ISO9660_PREPARATION
	$(INSTALL) -D -m 0644 $(ROOTFS_ISO9660_BOOT_MENU) \
		$(ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH)
	$(SED) "s%__KERNEL_PATH__%/boot/$(LINUX_IMAGE_NAME)%" \
		$(ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH)
	$(ROOTFS_ISO9660_INSTALL_BOOTLOADER_BIOS)
	$(ROOTFS_ISO9660_INSTALL_BOOTLOADER_EFI)
endef

ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_PREPARATION

define ROOTFS_ISO9660_DISABLE_EXTERNAL_INITRD
	$(SED) '/__INITRD_PATH__/d'  $(ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH)
endef

# Copy the kernel to temporary filesystem
define ROOTFS_ISO9660_COPY_KERNEL
	$(INSTALL) -D -m 0644 $(LINUX_IMAGE_PATH) \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)/boot/$(LINUX_IMAGE_NAME)
endef

ifeq ($(ROOTFS_ISO9660_USE_INITRD),YES)
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_COPY_KERNEL

# If initramfs is used, disable loading the initrd as the rootfs is
# already inside the kernel image. Otherwise, make sure a cpio is
# generated and use it as the initrd.
ifeq ($(LINGMO_TARGET_ROOTFS_INITRAMFS),y)
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_DISABLE_EXTERNAL_INITRD
else
ROOTFS_ISO9660_DEPENDENCIES += rootfs-cpio
define ROOTFS_ISO9660_COPY_INITRD
	$(INSTALL) -D -m 0644 $(BINARIES_DIR)/rootfs.cpio$(ROOTFS_CPIO_COMPRESS_EXT) \
		$(ROOTFS_ISO9660_TMP_TARGET_DIR)/boot/initrd
	$(SED) "s%__INITRD_PATH__%/boot/initrd%" \
		$(ROOTFS_ISO9660_BOOTLOADER_CONFIG_PATH)
endef
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_COPY_INITRD
endif

else # ROOTFS_ISO9660_USE_INITRD
ifeq ($(LINGMO_TARGET_ROOTFS_ISO9660_TRANSPARENT_COMPRESSION),y)
# We must use the uncompressed kernel image
ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_COPY_KERNEL
endif

ROOTFS_ISO9660_PRE_GEN_HOOKS += ROOTFS_ISO9660_DISABLE_EXTERNAL_INITRD

endif # ROOTFS_ISO9660_USE_INITRD

ROOTFS_ISO9660_OPTS += -J -R

ROOTFS_ISO9660_OPTS_BIOS = \
	-b $(ROOTFS_ISO9660_BOOT_IMAGE) \
	-no-emul-boot \
	-boot-load-size 4 \
	-boot-info-table

ROOTFS_ISO9660_OPTS_EFI = \
	--efi-boot $(ROOTFS_ISO9660_EFI_PARTITION) \
	-no-emul-boot

ifeq ($(LINGMO_TARGET_ROOTFS_ISO9660_BIOS_BOOTLOADER)$(LINGMO_TARGET_ROOTFS_ISO9660_EFI_BOOTLOADER),yy)
ROOTFS_ISO9660_OPTS += \
	$(ROOTFS_ISO9660_OPTS_BIOS) \
	-eltorito-alt-boot \
	$(ROOTFS_ISO9660_OPTS_EFI)
else ifeq ($(LINGMO_TARGET_ROOTFS_ISO9660_BIOS_BOOTLOADER),y)
ROOTFS_ISO9660_OPTS += $(ROOTFS_ISO9660_OPTS_BIOS)
else ifeq ($(LINGMO_TARGET_ROOTFS_ISO9660_EFI_BOOTLOADER),y)
ROOTFS_ISO9660_OPTS += $(ROOTFS_ISO9660_OPTS_EFI)
endif

define ROOTFS_ISO9660_CMD
	$(HOST_DIR)/bin/xorriso -as mkisofs \
		$(ROOTFS_ISO9660_OPTS) \
		-o $@ $(ROOTFS_ISO9660_TMP_TARGET_DIR)
endef

ifeq ($(LINGMO_TARGET_ROOTFS_ISO9660_HYBRID),y)
define ROOTFS_ISO9660_GEN_HYBRID
	$(HOST_DIR)/bin/isohybrid -t 0x96 $@
endef

ROOTFS_ISO9660_POST_GEN_HOOKS += ROOTFS_ISO9660_GEN_HYBRID
endif

$(eval $(rootfs))
