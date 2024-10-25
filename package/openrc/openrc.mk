################################################################################
#
# openrc
#
################################################################################

OPENRC_VERSION = 0.52.1
OPENRC_SITE = $(call github,OpenRC,openrc,$(OPENRC_VERSION))
OPENRC_LICENSE = BSD-2-Clause
OPENRC_LICENSE_FILES = LICENSE
OPENRC_CPE_ID_VALID = YES

OPENRC_DEPENDENCIES = ncurses

OPENRC_CONF_OPTS = \
	-Dos=Linux \
	-Dlibrcdir=/usr/libexec/rc \
	-Dpkgconfig=false \
	-Dsysvinit=true \
	-Drootprefix=/ \
	-Dbranding="\"Buildroot $(LINGMO_VERSION_FULL)\""

ifeq ($(LINGMO_PACKAGE_BASH_COMPLETION),y)
OPENRC_CONF_OPTS += -Dbash-completions=true
else
OPENRC_CONF_OPTS += -Dbash-completions=false
endif

ifeq ($(LINGMO_PACKAGE_LIBSELINUX),y)
OPENRC_CONF_OPTS += -Dselinux=enabled
OPENRC_DEPENDENCIES += libselinux
else
OPENRC_CONF_OPTS += -Dselinux=disabled
endif

ifeq ($(LINGMO_PACKAGE_LINUX_PAM),y)
OPENRC_CONF_OPTS += -Dpam=true
OPENRC_DEPENDENCIES += linux-pam
else
OPENRC_CONF_OPTS += -Dpam=false
endif

ifeq ($(LINGMO_ROOTFS_MERGED_USR),y)
OPENRC_CONF_OPTS += -Dsplit-usr=false
else
OPENRC_CONF_OPTS += -Dsplit-usr=true
endif

define OPENRC_INSTALL_SYSV_RCS_SCRIPT
	$(INSTALL) -D -m 0755 $(OPENRC_PKGDIR)/sysv-rcs \
		$(TARGET_DIR)/etc/init.d/sysv-rcs
endef
OPENRC_POST_INSTALL_TARGET_HOOKS += OPENRC_INSTALL_SYSV_RCS_SCRIPT

ifeq ($(LINGMO_PACKAGE_KBD),)
# keymaps and save-keymaps require kbd_mode and dumpkeys, respectively, so
# remove them if the kbd package is not selected (e.g. devices with serial
# console, only).
define OPENRC_NO_KBD
	$(RM) $(TARGET_DIR)/etc/runlevels/boot/{keymaps,save-keymaps}
	$(RM) $(TARGET_DIR)/etc/init.d/{keymaps,save-keymaps}
	$(RM) $(TARGET_DIR)/etc/conf.d/keymaps
endef
OPENRC_POST_INSTALL_TARGET_HOOKS += OPENRC_NO_KBD
endif

ifeq ($(LINGMO_PACKAGE_NETIFRC),y)
# netifrc replaces network, staticroute and loopback services which are
# installed by openrc
define OPENRC_NO_NET
	$(RM) $(TARGET_DIR)/etc/runlevels/boot/{network,staticroute,loopback}
	$(RM) $(TARGET_DIR)/etc/init.d/{network,staticroute,loopback}
	$(RM) $(TARGET_DIR)/etc/conf.d/{network,staticroute,loopback}
endef
OPENRC_POST_INSTALL_TARGET_HOOKS += OPENRC_NO_NET
endif

define OPENRC_REMOVE_UNNEEDED
	$(RM) -r $(TARGET_DIR)/usr/share/openrc
endef
OPENRC_TARGET_FINALIZE_HOOKS += OPENRC_REMOVE_UNNEEDED

ifeq ($(LINGMO_TARGET_GENERIC_GETTY),y)
OPENRC_GETTY_SVCNAME = agetty.$(SYSTEM_GETTY_PORT)
OPENRC_GETTY_CONF_D = $(TARGET_DIR)/etc/conf.d/$(OPENRC_GETTY_SVCNAME)
define OPENRC_SET_GETTY
	{ \
		echo "baud=\"$(SYSTEM_GETTY_BAUDRATE)\""; \
		echo "term_type=\"$(SYSTEM_GETTY_TERM)\"" ; \
		echo "agetty_options=\"-L $(SYSTEM_GETTY_OPTIONS)\""; \
	} > $(OPENRC_GETTY_CONF_D)
	ln -sf agetty $(TARGET_DIR)/etc/init.d/$(OPENRC_GETTY_SVCNAME)
	ln -sf /etc/init.d/$(OPENRC_GETTY_SVCNAME) \
		$(TARGET_DIR)/etc/runlevels/default/$(OPENRC_GETTY_SVCNAME)
endef
OPENRC_TARGET_FINALIZE_HOOKS += OPENRC_SET_GETTY
endif # LINGMO_TARGET_GENERIC_GETTY

$(eval $(meson-package))
