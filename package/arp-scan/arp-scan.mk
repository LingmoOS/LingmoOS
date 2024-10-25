################################################################################
#
# arp-scan
#
################################################################################

ARP_SCAN_VERSION = 1.10.0
ARP_SCAN_SITE = https://github.com/royhills/arp-scan/releases/download/$(ARP_SCAN_VERSION)
ARP_SCAN_LICENSE = GPL-3.0+
ARP_SCAN_LICENSE_FILES = COPYING
ARP_SCAN_DEPENDENCIES = libpcap

ifeq ($(LINGMO_STATIC_LIBS),y)
ARP_SCAN_CONF_OPTS += LIBS="`$(STAGING_DIR)/usr/bin/pcap-config --static --additional-libs`"
endif

ARP_SCAN_CONF_ENV = pgac_cv_snprintf_long_long_int_format='%lld'

ifeq ($(LINGMO_PACKAGE_LIBCAP),y)
ARP_SCAN_DEPENDENCIES += libcap
ARP_SCAN_CONF_OPTS += --with-libcap
else
ARP_SCAN_CONF_OPTS += --without-libcap
endif

ifeq ($(LINGMO_PACKAGE_ARP_SCAN_DATA),)
ARP_SCAN_INSTALL_TARGET_OPTS = DESTDIR=$(TARGET_DIR) install-exec
endif

$(eval $(autotools-package))
