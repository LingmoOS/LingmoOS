################################################################################
#
# gqrx
#
################################################################################

GQRX_VERSION = 2.15.9
GQRX_SITE = $(call github,csete,gqrx,v$(GQRX_VERSION))
GQRX_LICENSE = GPL-3.0+, Apache-2.0
GQRX_LICENSE_FILES = COPYING LICENSE-CTK
GQRX_DEPENDENCIES = boost gnuradio gr-osmosdr libsndfile qt5base qt5svg

GQRX_CONF_OPTS = -DLINUX_AUDIO_BACKEND=Gr-audio

# gqrx can use __atomic builtins, so we need to link with
# libatomic when available
ifeq ($(LINGMO_TOOLCHAIN_HAS_LIBATOMIC),y)
GQRX_CONF_OPTS += -DCMAKE_EXE_LINKER_FLAGS=-latomic
endif

$(eval $(cmake-package))
