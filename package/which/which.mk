################################################################################
#
# which
#
################################################################################

WHICH_VERSION = 2.21
WHICH_SITE = $(LINGMO_GNU_MIRROR)/which
WHICH_LICENSE = GPL-3.0+
WHICH_LICENSE_FILES = COPYING

$(eval $(autotools-package))
