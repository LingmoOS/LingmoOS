################################################################################
#
# gnuchess
#
################################################################################

GNUCHESS_VERSION = 6.2.9
GNUCHESS_SITE = $(LINGMO_GNU_MIRROR)/chess
GNUCHESS_LICENSE = GPL-3.0+
GNUCHESS_LICENSE_FILES = COPYING
GNUCHESS_CPE_ID_VENDOR = gnu
GNUCHESS_CPE_ID_PRODUCT = chess

GNUCHESS_DEPENDENCIES = host-flex flex
GNUCHESS_DEPENDENCIES += $(if $(LINGMO_PACKAGE_READLINE),readline) \
	$(TARGET_NLS_DEPENDENCIES)

$(eval $(autotools-package))
