################################################################################
#
# enlightenment
#
################################################################################

ENLIGHTENMENT_VERSION = 0.25.4
ENLIGHTENMENT_SOURCE = enlightenment-$(ENLIGHTENMENT_VERSION).tar.xz
ENLIGHTENMENT_SITE = https://download.enlightenment.org/rel/apps/enlightenment
ENLIGHTENMENT_LICENSE = BSD-2-Clause, OFL-1.1 (font)
ENLIGHTENMENT_LICENSE_FILES = COPYING \
	src/modules/wl_weekeyboard/themes/default/fonts/LICENSE.txt
ENLIGHTENMENT_CPE_ID_VENDOR = enlightenment

ENLIGHTENMENT_DEPENDENCIES = \
	host-pkgconf \
	host-efl \
	efl \
	xcb-util-keysyms

ENLIGHTENMENT_CONF_OPTS = \
	-Dedje-cc=$(HOST_DIR)/bin/edje_cc \
	-Deet=$(HOST_DIR)/bin/eet \
	-Deldbus-codegen=$(HOST_DIR)/bin/eldbus-codegen \
	-Dlibexif=false \
	-Dpam=false \
	-Dpolkit=false

# enlightenment.pc and /usr/lib/enlightenment/modules/*.so
ENLIGHTENMENT_INSTALL_STAGING = YES

ifeq ($(LINGMO_PACKAGE_SYSTEMD),y)
ENLIGHTENMENT_CONF_OPTS += -Dsystemd=true
ENLIGHTENMENT_DEPENDENCIES += systemd
else
ENLIGHTENMENT_CONF_OPTS += -Dsystemd=false
endif

# alsa backend needs mixer support
ifeq ($(LINGMO_PACKAGE_ALSA_LIB)$(LINGMO_PACKAGE_ALSA_LIB_MIXER),yy)
ENLIGHTENMENT_CONF_OPTS += -Dmixer=true
ENLIGHTENMENT_DEPENDENCIES += alsa-lib
else
ENLIGHTENMENT_CONF_OPTS += -Dmixer=false
endif

ifeq ($(LINGMO_PACKAGE_XKEYBOARD_CONFIG),y)
ENLIGHTENMENT_DEPENDENCIES += xkeyboard-config
endif

define ENLIGHTENMENT_REMOVE_DOCUMENTATION
	rm -rf $(TARGET_DIR)/usr/share/enlightenment/doc/
	rm -f $(TARGET_DIR)/usr/share/enlightenment/COPYING
	rm -f $(TARGET_DIR)/usr/share/enlightenment/AUTHORS
endef
ENLIGHTENMENT_POST_INSTALL_TARGET_HOOKS += ENLIGHTENMENT_REMOVE_DOCUMENTATION

$(eval $(meson-package))
