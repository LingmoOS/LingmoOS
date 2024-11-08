################################################################################
#
# luvi
#
################################################################################

LUVI_VERSION = 2.14.0
LUVI_SOURCE = luvi-src-v$(LUVI_VERSION).tar.gz
LUVI_SITE = https://github.com/luvit/luvi/releases/download/v$(LUVI_VERSION)
LUVI_LICENSE = Apache-2.0
LUVI_LICENSE_FILES = LICENSE.txt
LUVI_DEPENDENCIES = libuv luajit luv host-luajit host-pkgconf

# Dispatch all architectures of LuaJIT
ifeq ($(LINGMO_i386),y)
LUVI_TARGET_ARCH = x86
else ifeq ($(LINGMO_x86_64),y)
LUVI_TARGET_ARCH = x64
else ifeq ($(LINGMO_powerpc),y)
LUVI_TARGET_ARCH = ppc
else ifeq ($(LINGMO_arm)$(LINGMO_armeb),y)
LUVI_TARGET_ARCH = arm
else ifeq ($(LINGMO_aarch64),y)
LUVI_TARGET_ARCH = arm64
else ifeq ($(LINGMO_aarch64_be),y)
LUVI_TARGET_ARCH = arm64be
else ifeq ($(LINGMO_mips),y)
LUVI_TARGET_ARCH = mips
else ifeq ($(LINGMO_mipsel),y)
LUVI_TARGET_ARCH = mipsel
else
LUVI_TARGET_ARCH = $(LINGMO_ARCH)
endif

# LUAJIT_VERSION and the luajit installation path may not use the
# same value. Use the value from luajit.pc file.
LUVI_LUAJIT_MAJVER = `$(PKG_CONFIG_HOST_BINARY) --variable=majver luajit`
LUVI_LUAJIT_MINVER = `$(PKG_CONFIG_HOST_BINARY) --variable=minver luajit`

# Bundled lua bindings have to be linked statically into the luvi executable
LUVI_CONF_OPTS = \
	-DBUILD_SHARED_LIBS=OFF \
	-DWithSharedLibluv=ON \
	-DTARGET_ARCH=$(LUVI_TARGET_ARCH) \
	-DLUA_PATH=$(HOST_DIR)/share/luajit-$(LUVI_LUAJIT_MAJVER).$(LUVI_LUAJIT_MINVER)/?.lua

# Add "rex" module (PCRE via bundled lrexlib)
ifeq ($(LINGMO_PACKAGE_PCRE),y)
LUVI_DEPENDENCIES += pcre
LUVI_CONF_OPTS += -DWithPCRE=ON -DWithSharedPCRE=ON
else
LUVI_CONF_OPTS += -DWithPCRE=OFF -DWithSharedPCRE=OFF
endif

# Add "ssl" module (via bundled lua-openssl)
ifeq ($(LINGMO_PACKAGE_OPENSSL),y)
LUVI_DEPENDENCIES += openssl
LUVI_CONF_OPTS += -DWithOpenSSL=ON -DWithOpenSSLASM=ON -DWithSharedOpenSSL=ON
else
LUVI_CONF_OPTS += -DWithOpenSSL=OFF -DWithOpenSSLASM=OFF -DWithSharedOpenSSL=OFF
endif

# Add "zlib" module (via bundled lua-zlib)
ifeq ($(LINGMO_PACKAGE_ZLIB),y)
LUVI_DEPENDENCIES += zlib
LUVI_CONF_OPTS += -DWithZLIB=ON -DWithSharedZLIB=ON
else
LUVI_CONF_OPTS += -DWithZLIB=OFF -DWithSharedZLIB=OFF
endif

$(eval $(cmake-package))
