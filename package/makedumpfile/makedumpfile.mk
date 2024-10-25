################################################################################
#
# makedumpfile
#
################################################################################

MAKEDUMPFILE_VERSION = 1.7.4
MAKEDUMPFILE_SITE = https://github.com/makedumpfile/makedumpfile/releases/download/$(MAKEDUMPFILE_VERSION)
MAKEDUMPFILE_DEPENDENCIES = bzip2 elfutils xz zlib
MAKEDUMPFILE_LICENSE = GPL-2.0
MAKEDUMPFILE_LICENSE_FILES = COPYING

ifeq ($(LINGMO_powerpc),y)
MAKEDUMPFILE_TARGET = powerpc32
else
MAKEDUMPFILE_TARGET = $(LINGMO_ARCH)
endif

MAKEDUMPFILE_MAKE_OPTS = \
	CROSS= \
	TARGET=$(MAKEDUMPFILE_TARGET) \
	LINKTYPE=dynamic

ifeq ($(LINGMO_PACKAGE_LZO),y)
MAKEDUMPFILE_DEPENDENCIES += lzo
MAKEDUMPFILE_MAKE_OPTS += USELZO=on
endif

ifeq ($(LINGMO_PACKAGE_SNAPPY),y)
MAKEDUMPFILE_DEPENDENCIES += snappy
MAKEDUMPFILE_MAKE_OPTS += USESNAPPY=on
endif

ifeq ($(LINGMO_PACKAGE_ZSTD),y)
MAKEDUMPFILE_DEPENDENCIES += zstd
MAKEDUMPFILE_MAKE_OPTS += USEZSTD=on
endif

define MAKEDUMPFILE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		$(MAKEDUMPFILE_MAKE_OPTS)
endef

define MAKEDUMPFILE_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) install \
		DESTDIR="$(TARGET_DIR)"
endef

$(eval $(generic-package))
