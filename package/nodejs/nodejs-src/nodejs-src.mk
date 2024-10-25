################################################################################
#
# nodejs-src
#
################################################################################

NODEJS_SRC_VERSION = $(NODEJS_COMMON_VERSION)
NODEJS_SRC_SOURCE = $(NODEJS_COMMON_SOURCE)
NODEJS_SRC_SITE = $(NODEJS_COMMON_SITE)
NODEJS_SRC_DL_SUBDIR = nodejs

NODEJS_SRC_LICENSE = $(NODEJS_LICENSE)
NODEJS_SRC_LICENSE_FILES = $(NODEJS_LICENSE_FILES)
NODEJS_SRC_CPE_ID_VENDOR = $(NODEJS_CPE_ID_VENDOR)
NODEJS_SRC_CPE_ID_PRODUCT = $(NODEJS_CPE_ID_PRODUCT)

HOST_NODEJS_SRC_PROVIDES = host-nodejs

NODEJS_SRC_DEPENDENCIES = \
	host-ninja \
	host-pkgconf \
	host-python3 \
	host-qemu \
	c-ares \
	libuv \
	nghttp2 \
	zlib \
	$(call qstrip,$(LINGMO_PACKAGE_NODEJS_MODULES_ADDITIONAL_DEPS))
HOST_NODEJS_SRC_DEPENDENCIES = \
	host-icu \
	host-libopenssl \
	host-ninja \
	host-pkgconf \
	host-python3 \
	host-zlib
NODEJS_SRC_INSTALL_STAGING = YES

NODEJS_SRC_CONF_OPTS = \
	--shared-zlib \
	--shared-cares \
	--shared-libuv \
	--shared-nghttp2 \
	--cross-compiling \
	--dest-os=linux \
	--ninja

HOST_NODEJS_SRC_MAKE_OPTS = \
	$(HOST_CONFIGURE_OPTS) \
	CXXFLAGS="$(HOST_NODEJS_SRC_CXXFLAGS)" \
	LDFLAGS.host="$(HOST_LDFLAGS)" \
	NO_LOAD=cctest.target.mk \
	PATH=$(@D)/bin:$(BR_PATH) \
	JOBS=$(PARALLEL_JOBS)

NODEJS_SRC_MAKE_OPTS = \
	$(TARGET_CONFIGURE_OPTS) \
	NO_LOAD=cctest.target.mk \
	PATH=$(@D)/bin:$(BR_PATH) \
	LDFLAGS="$(NODEJS_SRC_LDFLAGS)" \
	LD="$(TARGET_CXX)" \
	JOBS=$(PARALLEL_JOBS)

# nodejs's build system uses python which can be a symlink to an unsupported
# python version (e.g. python 3.10 with nodejs 14.18.1). We work around this by
# forcing host-python3 early in the PATH, via a python->python3 symlink.
define NODEJS_SRC_PYTHON3_SYMLINK
	mkdir -p $(@D)/bin
	ln -sf $(HOST_DIR)/bin/python3 $(@D)/bin/python
endef
HOST_NODEJS_SRC_PRE_CONFIGURE_HOOKS += NODEJS_SRC_PYTHON3_SYMLINK
NODEJS_SRC_PRE_CONFIGURE_HOOKS += NODEJS_SRC_PYTHON3_SYMLINK

ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
NODEJS_SRC_DEPENDENCIES += openssl
NODEJS_SRC_CONF_OPTS += --shared-openssl
else
NODEJS_SRC_CONF_OPTS += --without-ssl
endif

ifeq ($(LINGMO_PACKAGE_ICU),y)
NODEJS_SRC_DEPENDENCIES += icu
NODEJS_SRC_CONF_OPTS += --with-intl=system-icu
else
NODEJS_SRC_CONF_OPTS += --with-intl=none
endif

ifneq ($(LINGMO_PACKAGE_NODEJS_NPM),y)
NODEJS_SRC_CONF_OPTS += --without-npm
endif

define HOST_NODEJS_SRC_CONFIGURE_CMDS
	cd $(@D); \
		$(HOST_CONFIGURE_OPTS) \
		PATH=$(@D)/bin:$(BR_PATH) \
		PYTHON=$(HOST_DIR)/bin/python3 \
		$(HOST_DIR)/bin/python3 configure.py \
		--prefix=$(HOST_DIR) \
		--shared-openssl \
		--shared-openssl-includes=$(HOST_DIR)/include \
		--shared-openssl-libpath=$(HOST_DIR)/lib \
		--shared-zlib \
		--no-cross-compiling \
		--with-intl=system-icu \
		--ninja
endef

HOST_NODEJS_SRC_CXXFLAGS = $(HOST_CXXFLAGS)

define HOST_NODEJS_SRC_BUILD_CMDS
	$(HOST_MAKE_ENV) PYTHON=$(HOST_DIR)/bin/python3 \
		$(MAKE) -C $(@D) \
		$(HOST_NODEJS_SRC_MAKE_OPTS)
endef

ifeq ($(LINGMO_PACKAGE_HOST_NODEJS_COREPACK),y)
define HOST_NODEJS_SRC_ENABLE_COREPACK
	$(COREPACK) enable
endef
endif

define HOST_NODEJS_SRC_INSTALL_CMDS
	$(HOST_MAKE_ENV) PYTHON=$(HOST_DIR)/bin/python3 \
		$(MAKE) -C $(@D) install \
		$(HOST_NODEJS_SRC_MAKE_OPTS)
	$(HOST_NODEJS_SRC_ENABLE_COREPACK)
endef

ifeq ($(LINGMO_i386),y)
NODEJS_SRC_CPU = ia32
else ifeq ($(LINGMO_x86_64),y)
NODEJS_SRC_CPU = x64
else ifeq ($(LINGMO_mips),y)
NODEJS_SRC_CPU = mips
else ifeq ($(LINGMO_mipsel),y)
NODEJS_SRC_CPU = mipsel
else ifeq ($(LINGMO_arm),y)
NODEJS_SRC_CPU = arm
# V8 needs to know what floating point ABI the target is using.
NODEJS_SRC_ARM_FP = $(GCC_TARGET_FLOAT_ABI)
# it also wants to know which FPU to use, but only has support for
# vfp, vfpv3, vfpv3-d16 and neon.
ifeq ($(LINGMO_ARM_FPU_VFPV2),y)
NODEJS_SRC_ARM_FPU = vfp
# vfpv4 is a superset of vfpv3
else ifeq ($(LINGMO_ARM_FPU_VFPV3)$(LINGMO_ARM_FPU_VFPV4),y)
NODEJS_SRC_ARM_FPU = vfpv3
# vfpv4-d16 is a superset of vfpv3-d16
else ifeq ($(LINGMO_ARM_FPU_VFPV3D16)$(LINGMO_ARM_FPU_VFPV4D16),y)
NODEJS_SRC_ARM_FPU = vfpv3-d16
else ifeq ($(LINGMO_ARM_FPU_NEON),y)
NODEJS_SRC_ARM_FPU = neon
endif
else ifeq ($(LINGMO_aarch64),y)
NODEJS_SRC_CPU = arm64
endif

# MIPS architecture specific options
ifeq ($(LINGMO_mips)$(LINGMO_mipsel),y)
ifeq ($(LINGMO_MIPS_CPU_MIPS32R6),y)
NODEJS_SRC_MIPS_ARCH_VARIANT = r6
NODEJS_SRC_MIPS_FPU_MODE = fp64
else ifeq ($(LINGMO_MIPS_CPU_MIPS32R2),y)
NODEJS_SRC_MIPS_ARCH_VARIANT = r2
else ifeq ($(LINGMO_MIPS_CPU_MIPS32),y)
NODEJS_SRC_MIPS_ARCH_VARIANT = r1
endif
endif

NODEJS_SRC_LDFLAGS = $(TARGET_LDFLAGS)

ifeq ($(LINGMO_TOOLCHAIN_HAS_LIBATOMIC),y)
NODEJS_SRC_LDFLAGS += -latomic
endif

# V8's JIT infrastructure requires binaries such as mksnapshot and
# mkpeephole to be run in the host during the build. However, these
# binaries must have the same bit-width as the target (e.g. a x86_64
# host targeting ARMv6 needs to produce a 32-bit binary). To work around this
# issue, cross-compile the binaries for the target and run them on the
# host with QEMU, much like gobject-introspection.
define NODEJS_SRC_INSTALL_V8_QEMU_WRAPPER
	$(INSTALL) -D -m 755 $(NODEJS_SRC_PKGDIR)/v8-qemu-wrapper.in \
		$(@D)/out/Release/v8-qemu-wrapper
	$(SED) "s%@QEMU_USER@%$(QEMU_USER)%g" \
		$(@D)/out/Release/v8-qemu-wrapper
	$(SED) "s%@TOOLCHAIN_HEADERS_VERSION@%$(LINGMO_TOOLCHAIN_HEADERS_AT_LEAST)%g" \
		$(@D)/out/Release/v8-qemu-wrapper
	$(SED) "s%@QEMU_USERMODE_ARGS@%$(call qstrip,$(LINGMO_PACKAGE_HOST_QEMU_USER_MODE_ARGS))%g" \
		$(@D)/out/Release/v8-qemu-wrapper
endef
NODEJS_SRC_PRE_CONFIGURE_HOOKS += NODEJS_SRC_INSTALL_V8_QEMU_WRAPPER

define NODEJS_SRC_WRAPPER_FIXUP
	$(SED) "s%@MAYBE_WRAPPER@%'<(PRODUCT_DIR)/v8-qemu-wrapper',%g" $(@D)/node.gyp
	$(SED) "s%@MAYBE_WRAPPER@%'<(PRODUCT_DIR)/v8-qemu-wrapper',%g" $(@D)/tools/v8_gypfiles/v8.gyp
endef
NODEJS_SRC_PRE_CONFIGURE_HOOKS += NODEJS_SRC_WRAPPER_FIXUP

# Do not run the qemu-wrapper for the host build.
define HOST_NODEJS_SRC_WRAPPER_FIXUP
	$(SED) "s%@MAYBE_WRAPPER@%%g" $(@D)/node.gyp
	$(SED) "s%@MAYBE_WRAPPER@%%g" $(@D)/tools/v8_gypfiles/v8.gyp
endef
HOST_NODEJS_SRC_PRE_CONFIGURE_HOOKS += HOST_NODEJS_SRC_WRAPPER_FIXUP

define NODEJS_SRC_CONFIGURE_CMDS
	(cd $(@D); \
		$(TARGET_CONFIGURE_OPTS) \
		PATH=$(@D)/bin:$(BR_PATH) \
		LDFLAGS="$(NODEJS_SRC_LDFLAGS)" \
		LD="$(TARGET_CXX)" \
		PYTHON=$(HOST_DIR)/bin/python3 \
		$(HOST_DIR)/bin/python3 configure.py \
		--prefix=/usr \
		--dest-cpu=$(NODEJS_SRC_CPU) \
		$(if $(NODEJS_SRC_ARM_FP),--with-arm-float-abi=$(NODEJS_SRC_ARM_FP)) \
		$(if $(NODEJS_SRC_ARM_FPU),--with-arm-fpu=$(NODEJS_SRC_ARM_FPU)) \
		$(if $(NODEJS_SRC_MIPS_ARCH_VARIANT),--with-mips-arch-variant=$(NODEJS_SRC_MIPS_ARCH_VARIANT)) \
		$(if $(NODEJS_SRC_MIPS_FPU_MODE),--with-mips-fpu-mode=$(NODEJS_SRC_MIPS_FPU_MODE)) \
		$(NODEJS_SRC_CONF_OPTS) \
	)
endef

define NODEJS_SRC_BUILD_CMDS
	$(TARGET_MAKE_ENV) PYTHON=$(HOST_DIR)/bin/python3 \
		$(MAKE) -C $(@D) \
		$(NODEJS_SRC_MAKE_OPTS)
endef

#
# Build the list of modules to install.
#
NODEJS_SRC_MODULES_LIST= $(call qstrip,\
	$(LINGMO_PACKAGE_NODEJS_MODULES_ADDITIONAL))

#
# We can only call NPM if there's something to install.
#
ifneq ($(NODEJS_SRC_MODULES_LIST),)
NODEJS_SRC_DEPENDENCIES += host-nodejs
define NODEJS_SRC_INSTALL_MODULES
	# If you're having trouble with module installation, adding -d to the
	# npm install call below and setting npm_config_rollback=false can both
	# help in diagnosing the problem.
	$(NPM) install -g $(NODEJS_SRC_MODULES_LIST)
endef

# Exclude prebuilt binaries with different architectures and OS from check
NODEJS_SRC_BIN_ARCH_EXCLUDE = /usr/lib/node_modules/
endif

define NODEJS_SRC_INSTALL_STAGING_CMDS
	$(TARGET_MAKE_ENV) PYTHON=$(HOST_DIR)/bin/python3 \
		$(MAKE) -C $(@D) install \
		DESTDIR=$(STAGING_DIR) \
		$(NODEJS_SRC_MAKE_OPTS)
endef

define NODEJS_SRC_INSTALL_TARGET_CMDS
	$(TARGET_MAKE_ENV) PYTHON=$(HOST_DIR)/bin/python3 \
		$(MAKE) -C $(@D) install \
		DESTDIR=$(TARGET_DIR) \
		$(NODEJS_SRC_MAKE_OPTS)
	$(NODEJS_SRC_INSTALL_MODULES)
endef

# node.js configure is a Python script and does not use autotools
$(eval $(generic-package))
$(eval $(host-generic-package))
