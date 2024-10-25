################################################################################
#
# htpdate
#
################################################################################

HTPDATE_VERSION = 1.3.7
HTPDATE_SITE = $(call github,twekkel,htpdate,v$(HTPDATE_VERSION))
HTPDATE_LICENSE = GPL-2.0+
HTPDATE_LICENSE_FILES = LICENSE

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
HTPDATE_BUILD_OPTS = \
	https \
	SSL_LIBS="`$(PKG_CONFIG_HOST_BINARY) --libs openssl`"
HTPDATE_DEPENDENCIES += openssl host-pkgconf
endif

define HTPDATE_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) $(HTPDATE_BUILD_OPTS)
endef

define HTPDATE_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(TARGET_CONFIGURE_OPTS) $(MAKE) -C $(@D) \
		DESTDIR=$(TARGET_DIR) install
endef

define HTPDATE_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 package/htpdate/S43htpdate \
		$(TARGET_DIR)/etc/init.d/S43htpdate
endef

define HTPDATE_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 package/htpdate/htpdate.service \
		$(TARGET_DIR)/usr/lib/systemd/system/htpdate.service
endef

$(eval $(generic-package))
