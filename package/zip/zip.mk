################################################################################
#
# zip
#
################################################################################

ZIP_VERSION = 3.0
# The version is really 3.0, but the tarball is named zip30.tar.gz
ZIP_SOURCE = zip$(subst .,,$(ZIP_VERSION)).tar.gz
ZIP_SITE = https://sourceforge.net/projects/infozip/files/Zip%203.x%20%28latest%29/$(ZIP_VERSION)
ZIP_LICENSE = Info-ZIP
ZIP_LICENSE_FILES = LICENSE
ZIP_CPE_ID_VENDOR = info-zip_project

ifeq ($(LINGMO_PACKAGE_BZIP2),y)
ZIP_DEPENDENCIES += bzip2
endif

# Infozip's default CFLAGS.
ZIP_CFLAGS = -I. -DUNIX

# Disable the support of 16-bit UIDs/GIDs, the test in unix/configure was
# removed since it can't work for cross-compilation.
ZIP_CFLAGS += -DUIDGID_NOT_16BIT

# infozip already defines _LARGEFILE_SOURCE and _LARGEFILE64_SOURCE when
# necessary, redefining it on the command line causes some warnings.
ZIP_TARGET_CFLAGS = \
	$(filter-out -D_LARGEFILE_SOURCE -D_LARGEFILE64_SOURCE,$(TARGET_CFLAGS))

define ZIP_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) \
		CFLAGS="$(ZIP_TARGET_CFLAGS) $(ZIP_CFLAGS)" \
		CC="$(TARGET_CC) -std=gnu89" AS="$(TARGET_CC) -c" \
		-f unix/Makefile generic
endef

define ZIP_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) -C $(@D) -f unix/Makefile install \
		prefix=$(TARGET_DIR)/usr
endef

define HOST_ZIP_BUILD_CMDS
	$(HOST_MAKE_ENV) $(MAKE) $(HOST_CONFIGURE_OPTS) -C $(@D) \
		CFLAGS="$(HOST_CFLAGS) $(ZIP_CFLAGS)" \
		CC="$(HOSTCC) -std=gnu89" AS="$(HOSTCC) -c" \
		-f unix/Makefile generic
endef

define HOST_ZIP_INSTALL_CMDS
	$(HOST_MAKE_ENV) $(MAKE) $(HOST_CONFIGURE_OPTS) -C $(@D) -f unix/Makefile install \
		prefix=$(HOST_DIR)
endef

$(eval $(generic-package))
$(eval $(host-generic-package))
