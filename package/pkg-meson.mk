################################################################################
# Meson package infrastructure
#
# This file implements an infrastructure that eases development of
# package .mk files for Meson packages. It should be used for all
# packages that use Meson as their build system.
#
# See the Buildroot documentation for details on the usage of this
# infrastructure
#
# In terms of implementation, this Meson infrastructure requires
# the .mk file to only specify metadata information about the
# package: name, version, download URL, etc.
#
# We still allow the package .mk file to override what the different
# steps are doing, if needed. For example, if <PKG>_BUILD_CMDS is
# already defined, it is used as the list of commands to perform to
# build the package, instead of the default Meson behaviour. The
# package can also define some post operation hooks.
#
################################################################################

#
# Pass PYTHONNOUSERSITE environment variable when invoking Meson or Ninja, so
# $(HOST_DIR)/bin/python3 will not look for Meson modules in
# $HOME/.local/lib/python3.x/site-packages
#
MESON		= PYTHONNOUSERSITE=y $(HOST_DIR)/bin/meson
NINJA		= PYTHONNOUSERSITE=y $(HOST_DIR)/bin/ninja
NINJA_OPTS	= $(if $(VERBOSE),-v)

# https://mesonbuild.com/Reference-tables.html#cpu-families
ifeq ($(LINGMO_arcle)$(LINGMO_arceb),y)
PKG_MESON_TARGET_CPU_FAMILY = arc
else ifeq ($(LINGMO_arm)$(LINGMO_armeb),y)
PKG_MESON_TARGET_CPU_FAMILY = arm
else ifeq ($(LINGMO_aarch64)$(LINGMO_aarch64_be),y)
PKG_MESON_TARGET_CPU_FAMILY = aarch64
else ifeq ($(LINGMO_i386),y)
PKG_MESON_TARGET_CPU_FAMILY = x86
else ifeq ($(LINGMO_m68k),y)
PKG_MESON_TARGET_CPU_FAMILY = m68k
else ifeq ($(LINGMO_microblazeel)$(LINGMO_microblazebe),y)
PKG_MESON_TARGET_CPU_FAMILY = microblaze
else ifeq ($(LINGMO_mips)$(LINGMO_mipsel),y)
PKG_MESON_TARGET_CPU_FAMILY = mips
else ifeq ($(LINGMO_mips64)$(LINGMO_mips64el),y)
PKG_MESON_TARGET_CPU_FAMILY = mips64
else ifeq ($(LINGMO_powerpc),y)
PKG_MESON_TARGET_CPU_FAMILY = ppc
else ifeq ($(LINGMO_powerpc64)$(LINGMO_powerpc64le),y)
PKG_MESON_TARGET_CPU_FAMILY = ppc64
else ifeq ($(LINGMO_riscv)$(LINGMO_RISCV_32),yy)
PKG_MESON_TARGET_CPU_FAMILY = riscv32
else ifeq ($(LINGMO_riscv)$(LINGMO_RISCV_64),yy)
PKG_MESON_TARGET_CPU_FAMILY = riscv64
else ifeq ($(LINGMO_s390x),y)
PKG_MESON_TARGET_CPU_FAMILY = s390x
else ifeq ($(LINGMO_sh4)$(LINGMO_sh4eb)$(LINGMO_sh4a)$(LINGMO_sh4aeb),y)
PKG_MESON_TARGET_CPU_FAMILY = sh4
else ifeq ($(LINGMO_sparc),y)
PKG_MESON_TARGET_CPU_FAMILY = sparc
else ifeq ($(LINGMO_sparc64),y)
PKG_MESON_TARGET_CPU_FAMILY = sparc64
else ifeq ($(LINGMO_x86_64),y)
PKG_MESON_TARGET_CPU_FAMILY = x86_64
else
PKG_MESON_TARGET_CPU_FAMILY = $(ARCH)
endif

# To avoid populating the cross-file with non existing compilers,
# we tie them to /bin/false
ifeq ($(LINGMO_INSTALL_LIBSTDCPP),y)
PKG_MESON_TARGET_CXX = $(TARGET_CXX)
else
PKG_MESON_TARGET_CXX = /bin/false
endif

ifeq ($(LINGMO_TOOLCHAIN_HAS_FORTRAN),y)
PKG_MESON_TARGET_FC = $(TARGET_FC)
else
PKG_MESON_TARGET_FC = /bin/false
endif

# Generates sed patterns for patching the cross-compilation.conf template,
# since Flags might contain commas the arguments are passed indirectly by
# variable name (stripped to deal with whitespaces).
# Arguments are variable containing cflags, cxxflags, ldflags, fcflags
define PKG_MESON_CROSSCONFIG_SED
        -e "s%@TARGET_CC@%$(TARGET_CC)%g" \
        -e "s%@TARGET_CXX@%$(PKG_MESON_TARGET_CXX)%g" \
        -e "s%@TARGET_AR@%$(TARGET_AR)%g" \
        -e "s%@TARGET_FC@%$(PKG_MESON_TARGET_FC)%g" \
        -e "s%@TARGET_STRIP@%$(TARGET_STRIP)%g" \
        -e "s%@TARGET_ARCH@%$(PKG_MESON_TARGET_CPU_FAMILY)%g" \
        -e "s%@TARGET_CPU@%$(GCC_TARGET_CPU)%g" \
        -e "s%@TARGET_ENDIAN@%$(call qstrip,$(call LOWERCASE,$(LINGMO_ENDIAN)))%g" \
        -e "s%@TARGET_FCFLAGS@%$(call make-sq-comma-list,$($(strip $(4))))%g" \
        -e "s%@TARGET_CFLAGS@%$(call make-sq-comma-list,$($(strip $(1))))%g" \
        -e "s%@TARGET_LDFLAGS@%$(call make-sq-comma-list,$($(strip $(3))))%g" \
        -e "s%@TARGET_CXXFLAGS@%$(call make-sq-comma-list,$($(strip $(2))))%g" \
        -e "s%@LINGMO_CMAKE@%$(LINGMO_CMAKE)%g" \
        -e "s%@PKGCONF_HOST_BINARY@%$(HOST_DIR)/bin/pkgconf%g" \
        -e "s%@HOST_DIR@%$(HOST_DIR)%g" \
        -e "s%@STAGING_DIR@%$(STAGING_DIR)%g" \
        -e "s%@STATIC@%$(if $(LINGMO_STATIC_LIBS),true,false)%g" \
        $(TOPDIR)/support/misc/cross-compilation.conf.in
endef

################################################################################
# inner-meson-package -- defines how the configuration, compilation and
# installation of a Meson package should be done, implements a few hooks to
# tune the build process and calls the generic package infrastructure to
# generate the necessary make targets
#
#  argument 1 is the lowercase package name
#  argument 2 is the uppercase package name, including a HOST_ prefix
#             for host packages
#  argument 3 is the uppercase package name, without the HOST_ prefix
#             for host packages
#  argument 4 is the type (target or host)
################################################################################

define inner-meson-package

#
# Configure step. Only define it if not already defined by the package
# .mk file. And take care of the differences between host and target
# packages.
#
ifndef $(2)_CONFIGURE_CMDS
ifeq ($(4),target)

$(2)_CFLAGS ?= $$(TARGET_CFLAGS)
$(2)_LDFLAGS ?= $$(TARGET_LDFLAGS)
$(2)_CXXFLAGS ?= $$(TARGET_CXXFLAGS)

# Configure package for target
#
#
define $(2)_CONFIGURE_CMDS
	rm -rf $$($$(PKG)_SRCDIR)/build
	mkdir -p $$($$(PKG)_SRCDIR)/build
	sed -e "/^\[binaries\]$$$$/s:$$$$:$$(foreach x,$$($(2)_MESON_EXTRA_BINARIES),\n$$(x)):" \
	    -e "/^\[properties\]$$$$/s:$$$$:$$(foreach x,$$($(2)_MESON_EXTRA_PROPERTIES),\n$$(x)):" \
	    $$(call PKG_MESON_CROSSCONFIG_SED,$(2)_CFLAGS,$(2)_CXXFLAGS,$(2)_LDFLAGS,$(2)_FCFLAGS) \
	    > $$($$(PKG)_SRCDIR)/build/cross-compilation.conf
	PATH=$$(BR_PATH) \
	CC_FOR_BUILD="$$(HOSTCC)" \
	CXX_FOR_BUILD="$$(HOSTCXX)" \
	$$($$(PKG)_CONF_ENV) \
	$$(MESON) setup \
		--prefix=/usr \
		--libdir=lib \
		--default-library=$(if $(LINGMO_STATIC_LIBS),static,shared) \
		--buildtype=$(if $(LINGMO_ENABLE_RUNTIME_DEBUG),debug,release) \
		--cross-file=$$($$(PKG)_SRCDIR)/build/cross-compilation.conf \
		-Db_pie=false \
		-Db_staticpic=$(if $(LINGMO_m68k_cf),false,true) \
		-Dstrip=false \
		-Dbuild.pkg_config_path=$$(HOST_DIR)/lib/pkgconfig \
		-Dbuild.cmake_prefix_path=$$(HOST_DIR)/lib/cmake \
		$$($$(PKG)_CONF_OPTS) \
		$$($$(PKG)_SRCDIR) $$($$(PKG)_SRCDIR)/build
endef
else

# Configure package for host
define $(2)_CONFIGURE_CMDS
	rm -rf $$($$(PKG)_SRCDIR)/build
	mkdir -p $$($$(PKG)_SRCDIR)/build
	$$(HOST_CONFIGURE_OPTS) \
	$$($$(PKG)_CONF_ENV) $$(MESON) setup \
		--prefix=$$(HOST_DIR) \
		--libdir=lib \
		--sysconfdir=$$(HOST_DIR)/etc \
		--localstatedir=$$(HOST_DIR)/var \
		--default-library=shared \
		--buildtype=release \
		--wrap-mode=nodownload \
		-Dstrip=true \
		$$($$(PKG)_CONF_OPTS) \
		$$($$(PKG)_SRCDIR) $$($$(PKG)_SRCDIR)/build
endef
endif
endif

$(2)_DEPENDENCIES += host-meson

#
# Build step. Only define it if not already defined by the package .mk
# file.
#
ifndef $(2)_BUILD_CMDS
ifeq ($(4),target)
define $(2)_BUILD_CMDS
	$$(TARGET_MAKE_ENV) $$($$(PKG)_NINJA_ENV) \
		$$(NINJA) $$(NINJA_OPTS) $$($$(PKG)_NINJA_OPTS) -C $$($$(PKG)_SRCDIR)/build
endef
else
define $(2)_BUILD_CMDS
	$$(HOST_MAKE_ENV) $$($$(PKG)_NINJA_ENV) \
		$$(NINJA) $$(NINJA_OPTS) $$($$(PKG)_NINJA_OPTS) -C $$($$(PKG)_SRCDIR)/build
endef
endif
endif

#
# Host installation step. Only define it if not already defined by the
# package .mk file.
#
ifndef $(2)_INSTALL_CMDS
define $(2)_INSTALL_CMDS
	$$(HOST_MAKE_ENV) $$($$(PKG)_NINJA_ENV) \
		$$(NINJA) $$(NINJA_OPTS) -C $$($$(PKG)_SRCDIR)/build install
endef
endif

#
# Staging installation step. Only define it if not already defined by
# the package .mk file.
#
ifndef $(2)_INSTALL_STAGING_CMDS
define $(2)_INSTALL_STAGING_CMDS
	$$(TARGET_MAKE_ENV) $$($$(PKG)_NINJA_ENV) DESTDIR=$$(STAGING_DIR) \
		$$(NINJA) $$(NINJA_OPTS) -C $$($$(PKG)_SRCDIR)/build install
endef
endif

#
# Target installation step. Only define it if not already defined by
# the package .mk file.
#
ifndef $(2)_INSTALL_TARGET_CMDS
define $(2)_INSTALL_TARGET_CMDS
	$$(TARGET_MAKE_ENV) $$($$(PKG)_NINJA_ENV) DESTDIR=$$(TARGET_DIR) \
		$$(NINJA) $$(NINJA_OPTS) -C $$($$(PKG)_SRCDIR)/build install
endef
endif

# Call the generic package infrastructure to generate the necessary
# make targets
$(call inner-generic-package,$(1),$(2),$(3),$(4))

endef

################################################################################
# meson-package -- the target generator macro for Meson packages
################################################################################

meson-package = $(call inner-meson-package,$(pkgname),$(call UPPERCASE,$(pkgname)),$(call UPPERCASE,$(pkgname)),target)
host-meson-package = $(call inner-meson-package,host-$(pkgname),$(call UPPERCASE,host-$(pkgname)),$(call UPPERCASE,$(pkgname)),host)

################################################################################
# Generation of the Meson compile flags and cross-compilation file
################################################################################

# Generate a Meson cross-compilation.conf suitable for use with the
# SDK; also install the file as a template for users to add their
# own flags if they need to.
define PKG_MESON_INSTALL_CROSS_CONF
	mkdir -p $(HOST_DIR)/etc/meson
	sed -e "s%@TARGET_CFLAGS@%$(call make-sq-comma-list,$(TARGET_CFLAGS))@PKG_TARGET_CFLAGS@%g" \
	    -e "s%@TARGET_LDFLAGS@%$(call make-sq-comma-list,$(TARGET_LDFLAGS))@PKG_TARGET_LDFLAGS@%g" \
	    -e "s%@TARGET_CXXFLAGS@%$(call make-sq-comma-list,$(TARGET_CXXFLAGS))@PKG_TARGET_CXXFLAGS@%g" \
	    -e "s%@TARGET_FCFLAGS@%$(call make-sq-comma-list,$(TARGET_FCFLAGS))@PKG_TARGET_FCFLAGS@%g" \
	    $(call PKG_MESON_CROSSCONFIG_SED) \
	    > $(HOST_DIR)/etc/meson/cross-compilation.conf.in
	sed $(call PKG_MESON_CROSSCONFIG_SED,TARGET_CFLAGS,TARGET_CXXFLAGS,TARGET_LDFLAGS,TARGET_FCFLAGS) \
	    > $(HOST_DIR)/etc/meson/cross-compilation.conf
endef

TOOLCHAIN_TARGET_FINALIZE_HOOKS += PKG_MESON_INSTALL_CROSS_CONF
