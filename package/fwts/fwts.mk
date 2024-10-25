################################################################################
#
# fwts
#
################################################################################

FWTS_VERSION = 23.07.00
FWTS_SOURCE = fwts-V$(FWTS_VERSION).tar.gz
FWTS_SITE = https://fwts.ubuntu.com/release
FWTS_STRIP_COMPONENTS = 0
FWTS_LICENSE = GPL-2.0, LGPL-2.1, Custom
FWTS_LICENSE_FILES = debian/copyright
FWTS_AUTORECONF = YES
FWTS_DEPENDENCIES = host-bison host-flex host-pkgconf libglib2 libbsd \
	$(if $(LINGMO_PACKAGE_BASH_COMPLETION),bash-completion) \
	$(if $(LINGMO_PACKAGE_DTC),dtc)

ifeq ($(LINGMO_OPTIMIZE_0),y)
FWTS_CONF_ENV = CFLAGS="$(TARGET_CFLAGS) -O1"
endif

ifeq ($(LINGMO_PACKAGE_FWTS_EFI_RUNTIME_MODULE),y)
FWTS_MODULE_SUBDIRS = efi_runtime
$(eval $(kernel-module))
endif

$(eval $(autotools-package))
