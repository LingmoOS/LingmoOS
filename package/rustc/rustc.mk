################################################################################
#
# rustc
#
################################################################################

RUSTC_ARCH = $(call qstrip,$(LINGMO_PACKAGE_HOST_RUSTC_ARCH))
RUSTC_ABI = $(call qstrip,$(LINGMO_PACKAGE_HOST_RUSTC_ABI))

ifeq ($(LINGMO_PACKAGE_HOST_RUSTC_TARGET_ARCH_SUPPORTS),y)
RUSTC_TARGET_NAME = $(RUSTC_ARCH)-unknown-linux-$(LIBC)$(RUSTC_ABI)
endif

ifeq ($(HOSTARCH),riscv64)
RUSTC_HOST_ARCH = riscv64gc
else ifeq ($(HOSTARCH),x86)
RUSTC_HOST_ARCH = i686
else
RUSTC_HOST_ARCH = $(HOSTARCH)
endif

RUSTC_HOST_NAME = $(RUSTC_HOST_ARCH)-unknown-linux-gnu

$(eval $(host-virtual-package))
