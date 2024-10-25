################################################################################
#
# Build the jffs2 root filesystem image
#
################################################################################

JFFS2_OPTS = -e $(LINGMO_TARGET_ROOTFS_JFFS2_EBSIZE) --with-xattr
SUMTOOL_OPTS = -e $(LINGMO_TARGET_ROOTFS_JFFS2_EBSIZE)

ifeq ($(LINGMO_TARGET_ROOTFS_JFFS2_PAD),y)
ifneq ($(strip $(LINGMO_TARGET_ROOTFS_JFFS2_PADSIZE)),0x0)
JFFS2_OPTS += --pad=$(strip $(LINGMO_TARGET_ROOTFS_JFFS2_PADSIZE))
else
JFFS2_OPTS += -p
endif
SUMTOOL_OPTS += -p
endif

ifeq ($(LINGMO_TARGET_ROOTFS_JFFS2_LE),y)
JFFS2_OPTS += -l
SUMTOOL_OPTS += -l
endif

ifeq ($(LINGMO_TARGET_ROOTFS_JFFS2_BE),y)
JFFS2_OPTS += -b
SUMTOOL_OPTS += -b
endif

ifeq ($(LINGMO_TARGET_ROOTFS_JFFS2_USE_CUSTOM_PAGESIZE),y)
JFFS2_OPTS += -s $(LINGMO_TARGET_ROOTFS_JFFS2_CUSTOM_PAGESIZE)
endif

ifeq ($(LINGMO_TARGET_ROOTFS_JFFS2_NOCLEANMARKER),y)
JFFS2_OPTS += -n
SUMTOOL_OPTS += -n
endif

ROOTFS_JFFS2_DEPENDENCIES = host-mtd

ifneq ($(LINGMO_TARGET_ROOTFS_JFFS2_SUMMARY),)
define ROOTFS_JFFS2_CMD
	$(MKFS_JFFS2) $(JFFS2_OPTS) -d $(TARGET_DIR) -o $@.nosummary
	$(SUMTOOL) $(SUMTOOL_OPTS) -i $@.nosummary -o $@
	rm $@.nosummary
endef
else
define ROOTFS_JFFS2_CMD
	$(MKFS_JFFS2) $(JFFS2_OPTS) -d $(TARGET_DIR) -o $@
endef
endif

$(eval $(rootfs))
