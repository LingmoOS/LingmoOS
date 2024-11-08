################################################################################
#
# ljsyscall
#
################################################################################

LJSYSCALL_VERSION = 0.12
LJSYSCALL_SITE = $(call github,justincormack,ljsyscall,v$(LJSYSCALL_VERSION))
LJSYSCALL_LICENSE = MIT
LJSYSCALL_LICENSE_FILES = COPYRIGHT

# dispatch all architectures of LuaJIT
ifeq ($(LINGMO_i386),y)
LJSYSCALL_ARCH = x86
else ifeq ($(LINGMO_x86_64),y)
LJSYSCALL_ARCH = x64
else ifeq ($(LINGMO_powerpc),y)
LJSYSCALL_ARCH = ppc
else ifeq ($(LINGMO_arm)$(LINGMO_armeb),y)
LJSYSCALL_ARCH = arm
else ifeq ($(LINGMO_aarch64)$(LINGMO_aarch64_be),y)
LJSYSCALL_ARCH = arm64
else ifeq ($(LINGMO_mips)$(LINGMO_mipsel),y)
LJSYSCALL_ARCH = mips
else
LJSYSCALL_ARCH = $(LINGMO_ARCH)
endif

LJSYSCALL_TARGET_DIR = $(TARGET_DIR)/usr/share/lua/$(LUAINTERPRETER_ABIVER)

define LJSYSCALL_INSTALL_TARGET_CMDS
	$(INSTALL) -d $(LJSYSCALL_TARGET_DIR)/syscall
	$(INSTALL) -m 0644 -t $(LJSYSCALL_TARGET_DIR)/ $(@D)/syscall.lua
	$(INSTALL) -m 0644 -t $(LJSYSCALL_TARGET_DIR)/syscall $(@D)/syscall/*.lua

	$(INSTALL) -d $(LJSYSCALL_TARGET_DIR)/syscall/linux/$(LJSYSCALL_ARCH)
	$(INSTALL) -m 0644 -t $(LJSYSCALL_TARGET_DIR)/syscall/linux/ $(@D)/syscall/linux/*.lua
	$(INSTALL) -m 0644 -t $(LJSYSCALL_TARGET_DIR)/syscall/linux/$(LJSYSCALL_ARCH) $(@D)/syscall/linux/$(LJSYSCALL_ARCH)/*.lua

	$(INSTALL) -d $(LJSYSCALL_TARGET_DIR)/syscall/shared
	$(INSTALL) -m 0644 -t $(LJSYSCALL_TARGET_DIR)/syscall/shared/ $(@D)/syscall/shared/*.lua
endef

$(eval $(generic-package))
