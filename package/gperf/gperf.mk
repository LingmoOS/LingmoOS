################################################################################
#
# gperf
#
################################################################################

GPERF_VERSION = 3.1
GPERF_SITE = $(LINGMO_GNU_MIRROR)/gperf
GPERF_LICENSE = GPL-3.0+
GPERF_LICENSE_FILES = COPYING

$(eval $(autotools-package))
$(eval $(host-autotools-package))
