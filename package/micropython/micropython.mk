################################################################################
#
# micropython
#
################################################################################

MICROPYTHON_VERSION = 1.22.0
MICROPYTHON_SITE = https://micropython.org/resources/source
MICROPYTHON_SOURCE = micropython-$(MICROPYTHON_VERSION).tar.xz
# Micropython has a lot of code copied from other projects, and also a number
# of submodules for various libs. However, we don't even clone the submodules,
# and most of the copied code is not used in the unix build.
MICROPYTHON_LICENSE = MIT, BSD-1-Clause, BSD-3-Clause, Zlib
MICROPYTHON_LICENSE_FILES = LICENSE
MICROPYTHON_DEPENDENCIES = host-python3
MICROPYTHON_CPE_ID_VENDOR = micropython

# Use fallback implementation for exception handling on architectures that don't
# have explicit support.
ifeq ($(LINGMO_i386)$(LINGMO_x86_64)$(LINGMO_arm)$(LINGMO_armeb),)
MICROPYTHON_CFLAGS = -DMICROPY_GCREGS_SETJMP=1
endif

# xtensa has problems with nlr_push, use setjmp based implementation instead
ifeq ($(LINGMO_xtensa),y)
MICROPYTHON_CFLAGS = -DMICROPY_NLR_SETJMP=1
endif

# When building from a tarball we don't have some of the dependencies that are in
# the git repository as submodules
MICROPYTHON_MAKE_OPTS += \
	MICROPY_PY_BTREE=0 \
	MICROPY_PY_USSL=0 \
	CROSS_COMPILE=$(TARGET_CROSS) \
	CFLAGS_EXTRA=$(MICROPYTHON_CFLAGS) \
	LDFLAGS_EXTRA="$(TARGET_LDFLAGS)" \
	CWARN=

ifeq ($(LINGMO_PACKAGE_LIBFFI),y)
MICROPYTHON_DEPENDENCIES += host-pkgconf libffi
MICROPYTHON_MAKE_OPTS += MICROPY_PY_FFI=1
else
MICROPYTHON_MAKE_OPTS += MICROPY_PY_FFI=0
endif

define MICROPYTHON_BUILD_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/mpy-cross
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/ports/unix \
		$(MICROPYTHON_MAKE_OPTS)
endef

define MICROPYTHON_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) $(MAKE) -C $(@D)/ports/unix \
		$(MICROPYTHON_MAKE_OPTS) \
		DESTDIR=$(TARGET_DIR) \
		PREFIX=/usr \
		install
endef

ifeq ($(LINGMO_PACKAGE_MICROPYTHON_LIB),y)
define MICROPYTHON_COLLECT_LIBS
	$(EXTRA_ENV) PYTHONPATH=$(@D)/tools \
		package/micropython/collect_micropython_lib.py \
		$(@D) $(@D)/.built_pylib
endef

define MICROPYTHON_INSTALL_LIBS
	$(INSTALL) -d -m 0755 $(TARGET_DIR)/usr/lib/micropython
	cp -a $(@D)/.built_pylib/* $(TARGET_DIR)/usr/lib/micropython
endef

MICROPYTHON_POST_BUILD_HOOKS += MICROPYTHON_COLLECT_LIBS
MICROPYTHON_POST_INSTALL_TARGET_HOOKS += MICROPYTHON_INSTALL_LIBS
endif

$(eval $(generic-package))
