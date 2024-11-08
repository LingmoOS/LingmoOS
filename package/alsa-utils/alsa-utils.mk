################################################################################
#
# alsa-utils
#
################################################################################

ALSA_UTILS_VERSION = 1.2.11
ALSA_UTILS_SOURCE = alsa-utils-$(ALSA_UTILS_VERSION).tar.bz2
ALSA_UTILS_SITE = https://www.alsa-project.org/files/pub/utils
ALSA_UTILS_LICENSE = GPL-2.0
ALSA_UTILS_LICENSE_FILES = COPYING
# 0011-configure.ac-fix-UMP-support-detection.patch
ALSA_UTILS_AUTORECONF = YES
ALSA_UTILS_INSTALL_STAGING = YES
ALSA_UTILS_DEPENDENCIES = host-pkgconf alsa-lib \
	$(if $(LINGMO_PACKAGE_NCURSES),ncurses) \
	$(if $(LINGMO_PACKAGE_LIBSAMPLERATE),libsamplerate) \
	$(TARGET_NLS_DEPENDENCIES)

ifeq ($(LINGMO_PACKAGE_ALSA_UTILS_ALSACTL),y)
ALSA_UTILS_SELINUX_MODULES += alsa
endif

ALSA_UTILS_CONF_ENV = \
	ac_cv_prog_ncurses5_config=$(STAGING_DIR)/usr/bin/$(NCURSES_CONFIG_SCRIPTS) \
	LIBS=$(TARGET_NLS_LIBS)

ALSA_UTILS_CONF_OPTS = \
	--disable-xmlto \
	--disable-rst2man \
	--with-curses=$(if $(LINGMO_PACKAGE_NCURSES_WCHAR),ncursesw,ncurses)

ifeq ($(LINGMO_PACKAGE_ALSA_UTILS_ALSALOOP),y)
ALSA_UTILS_CONF_OPTS += --enable-alsaloop
else
ALSA_UTILS_CONF_OPTS += --disable-alsaloop
endif

ifneq ($(LINGMO_PACKAGE_ALSA_UTILS_ALSAMIXER),y)
ALSA_UTILS_CONF_OPTS += --disable-alsamixer
endif

ifeq ($(LINGMO_PACKAGE_ALSA_UTILS_BAT),y)
ALSA_UTILS_CONF_OPTS += --enable-bat
# Analysis support requires fftw single precision
ALSA_UTILS_DEPENDENCIES += $(if $(LINGMO_PACKAGE_FFTW_SINGLE),fftw-single)
else
ALSA_UTILS_CONF_OPTS += --disable-bat
endif

ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_ALSACONF) += usr/sbin/alsaconf
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_ALSACTL) += usr/sbin/alsactl
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_ALSALOOP) += usr/bin/alsaloop
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_ALSAMIXER) += usr/bin/alsamixer
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_ALSATPLG) += usr/bin/alsatplg
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_AMIDI) += usr/bin/amidi
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_AMIXER) += usr/bin/amixer
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_APLAY) += usr/bin/aplay usr/bin/arecord
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_BAT) += usr/bin/alsabat
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_IECSET) += usr/bin/iecset
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_ACONNECT) += usr/bin/aconnect
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_ALSAUCM) += usr/bin/alsaucm
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_APLAYMIDI) += usr/bin/aplaymidi
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_ARECORDMIDI) += usr/bin/arecordmidi
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_ASEQDUMP) += usr/bin/aseqdump
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_ASEQNET) += usr/bin/aseqnet
ALSA_UTILS_TARGETS_$(LINGMO_PACKAGE_ALSA_UTILS_SPEAKER_TEST) += usr/bin/speaker-test

define ALSA_UTILS_INSTALL_TARGET_CMDS
	mkdir -p $(TARGET_DIR)/var/lib/alsa
	for i in $(ALSA_UTILS_TARGETS_y); do \
		$(INSTALL) -D -m 755 $(STAGING_DIR)/$$i $(TARGET_DIR)/$$i || exit 1; \
	done
	if [ -x "$(TARGET_DIR)/usr/bin/speaker-test" ]; then \
		mkdir -p $(TARGET_DIR)/usr/share/alsa/speaker-test; \
		mkdir -p $(TARGET_DIR)/usr/share/sounds/alsa; \
		cp -rdpf $(STAGING_DIR)/usr/share/alsa/speaker-test/* $(TARGET_DIR)/usr/share/alsa/speaker-test/; \
		cp -rdpf $(STAGING_DIR)/usr/share/sounds/alsa/* $(TARGET_DIR)/usr/share/sounds/alsa/; \
	fi
	if [ -x "$(TARGET_DIR)/usr/sbin/alsactl" ]; then \
		mkdir -p $(TARGET_DIR)/usr/share/; \
		cp -rdpf $(STAGING_DIR)/usr/share/alsa/* $(TARGET_DIR)/usr/share/alsa/; \
	fi
endef

ifeq ($(LINGMO_PACKAGE_ALSA_UTILS_ALSACTL)$(LINGMO_INIT_SYSTEMD),yy)
ALSA_UTILS_DEPENDENCIES += systemd
ALSA_UTILS_CONF_OPTS += --with-systemdsystemunitdir=/usr/lib/systemd/system
define ALSA_UTILS_INSTALL_INIT_SYSTEMD
	$(INSTALL) -D -m 0644 $(@D)/alsactl/alsa-restore.service \
		$(TARGET_DIR)/usr/lib/systemd/system/alsa-restore.service
	$(INSTALL) -D -m 0644 $(@D)/alsactl/alsa-state.service \
		$(TARGET_DIR)/usr/lib/systemd/system/alsa-state.service
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/usr/lib/systemd/system/alsa-restore.service.d
	printf '[Install]\nWantedBy=multi-user.target\n' \
		>$(TARGET_DIR)/usr/lib/systemd/system/alsa-restore.service.d/buildroot-enable.conf
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/usr/lib/systemd/system/alsa-state.service.d
	printf '[Install]\nWantedBy=multi-user.target\n' \
		>$(TARGET_DIR)/usr/lib/systemd/system/alsa-state.service.d/buildroot-enable.conf;
endef
endif

$(eval $(autotools-package))
