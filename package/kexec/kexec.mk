################################################################################
#
# kexec
#
################################################################################

KEXEC_VERSION = 2.0.28
KEXEC_SOURCE = kexec-tools-$(KEXEC_VERSION).tar.xz
KEXEC_SITE = $(LINGMO_KERNEL_MIRROR)/linux/utils/kernel/kexec
KEXEC_LICENSE = GPL-2.0
KEXEC_LICENSE_FILES = COPYING
KEXEC_CPE_ID_PRODUCT = kexec-tools
KEXEC_CPE_ID_VENDOR = kexec-tools_project
KEXEC_SELINUX_MODULES = kdump

# Makefile expects $STRIP -o to work, so needed for !LINGMO_STRIP_strip
KEXEC_MAKE_OPTS = STRIP="$(TARGET_CROSS)strip"

ifeq ($(LINGMO_PACKAGE_KEXEC_ZLIB),y)
KEXEC_CONF_OPTS += --with-zlib
KEXEC_DEPENDENCIES += zlib
else
KEXEC_CONF_OPTS += --without-zlib
endif

ifeq ($(LINGMO_PACKAGE_XZ),y)
KEXEC_CONF_OPTS += --with-lzma
KEXEC_DEPENDENCIES += xz
else
KEXEC_CONF_OPTS += --without-lzma
endif

define KEXEC_REMOVE_LIB_TOOLS
	rm -rf $(TARGET_DIR)/usr/lib/kexec-tools
endef

KEXEC_POST_INSTALL_TARGET_HOOKS += KEXEC_REMOVE_LIB_TOOLS

$(eval $(autotools-package))
