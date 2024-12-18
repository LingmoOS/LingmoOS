################################################################################
#
# acpid
#
################################################################################

ACPID_VERSION = 2.0.34
ACPID_SOURCE = acpid-$(ACPID_VERSION).tar.xz
ACPID_SITE = http://downloads.sourceforge.net/project/acpid2
ACPID_LICENSE = GPL-2.0+
ACPID_LICENSE_FILES = COPYING
ACPID_CPE_ID_VENDOR = tedfelix
ACPID_CPE_ID_PRODUCT = acpid2
ACPID_SELINUX_MODULES = acpi

define ACPID_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 package/acpid/acpid.service \
		$(TARGET_DIR)/usr/lib/systemd/system/acpid.service
endef

define ACPID_INSTALL_INIT_SYSV
	$(INSTALL) -D -m 0755 package/acpid/S02acpid \
		$(TARGET_DIR)/etc/init.d/S02acpid
endef

ifeq ($(LINGMO_INIT_SYSV)$(LINGMO_INIT_SYSTEMD),y)
ACPID_POWEROFF_CMD = /sbin/shutdown -hP now
else
ACPID_POWEROFF_CMD = /sbin/poweroff
endif

define ACPID_SET_EVENTS
	mkdir -p $(TARGET_DIR)/etc/acpi/events
	printf 'event=button[ /]power\naction=%s\n' '$(ACPID_POWEROFF_CMD)' \
		>$(TARGET_DIR)/etc/acpi/events/powerbtn
endef

ACPID_POST_INSTALL_TARGET_HOOKS += ACPID_SET_EVENTS

$(eval $(autotools-package))
