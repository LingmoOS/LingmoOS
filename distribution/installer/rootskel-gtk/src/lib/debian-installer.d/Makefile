dir = lib/debian-installer.d

DEB_HOST_ARCH_OS    := $(shell dpkg-architecture -qDEB_HOST_ARCH_OS 2>/dev/null)

ifeq ($(DEB_HOST_ARCH_OS),)
  DEB_HOST_ARCH_OS    := $(shell dpkg-architecture -qDEB_HOST_GNU_SYSTEM)
endif

files = \
	S62Xorg

ifeq ($(DEB_HOST_ARCH_CPU),linux)
   files += \
	S61mouse-support-linux
endif

include ../../../Makefile.inc
