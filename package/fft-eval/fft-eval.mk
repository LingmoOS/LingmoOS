################################################################################
#
# fft-eval
#
################################################################################

FFT_EVAL_VERSION = 6a82b383ce79afc4c1ba26ffcf34b15d3e95894b
FFT_EVAL_SITE = $(call github,simonwunderlich,FFT_eval,$(FFT_EVAL_VERSION))
FFT_EVAL_LICENSE = CC0-1.0 (doc), GPL-2.0, OFL-1.1 (LiberationSans-Regular.ttf)
FFT_EVAL_LICENSE_FILES = \
	LICENSES/CC0-1.0.txt \
	LICENSES/GPL-2.0-only.txt \
	LICENSES/OFL-1.1.txt

FFT_EVAL_CONV_OPTS = CONFIG_fft_eval_json=y

ifeq ($(LINGMO_PACKAGE_SDL2)$(LINGMO_PACKAGE_SDL2_TTF),yy)
FFT_EVAL_CONV_OPTS += CONFIG_fft_eval_sdl=y
FFT_EVAL_DEPENDENCIES += sdl2 sdl2_ttf
else
FFT_EVAL_CONV_OPTS += CONFIG_fft_eval_sdl=n
endif

define FFT_EVAL_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) \
		CROSS=$(STAGING_DIR)/usr/bin/ $(FFT_EVAL_CONV_OPTS) -C $(@D) all
endef

define FFT_EVAL_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) $(TARGET_CONFIGURE_OPTS) \
		CROSS=$(STAGING_DIR)/usr/bin/ $(FFT_EVAL_CONV_OPTS) \
		-C $(@D) DESTDIR=$(TARGET_DIR) BINDIR=/usr/bin install
endef

$(eval $(generic-package))
