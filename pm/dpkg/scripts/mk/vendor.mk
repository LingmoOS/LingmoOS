# This Makefile fragment (since dpkg 1.16.1) defines the following
# vendor-related variables:
#
#   DEB_VENDOR: Output of «dpkg-vendor --query Vendor».
#   DEB_PARENT_VENDOR: Output of «dpkg-vendor --query Parent» (can be empty).
#
# This Makefile fragment also defines a set of "dpkg_vendor_derives_from"
# macros that can be used to verify if the current vendor derives from
# another vendor. The unversioned variant defaults to the v0 version if
# undefined or to the v1 version when on build API 1, otherwise it can be
# defined explicitly to one of the versions or the versioned macros can be
# used directly. The following are example usages:
#
# - dpkg_vendor_derives_from (since dpkg 1.16.1)
#
#   ifeq ($(shell $(call dpkg_vendor_derives_from,ubuntu)),yes)
#     ...
#   endif
#
# - dpkg_vendor_derives_from_v0 (since dpkg 1.19.3)
#
#   ifeq ($(shell $(call dpkg_vendor_derives_from_v0,ubuntu)),yes)
#     ...
#   endif
#
# - dpkg_vendor_derives_from_v1 (since dpkg 1.19.3)
#
#   dpkg_vendor_derives_from = $(dpkg_vendor_derives_from_v1)
#   ifeq ($(call dpkg_vendor_derives_from,ubuntu),yes)
#     ...
#   endif
#   ifeq ($(call dpkg_vendor_derives_from_v1,ubuntu),yes)
#     ...
#   endif
#
# Note:
# - Only documented variables are considered public interfaces.
# - Expects to be included from the source tree root directory.

ifndef dpkg_vendor_mk_included
dpkg_vendor_mk_included = yes

ifndef dpkg_datadir
  dpkg_datadir := $(dir $(lastword $(MAKEFILE_LIST)))
endif
include $(dpkg_datadir)/buildapi.mk

dpkg_late_eval ?= $(if $(filter undefined,$(flavor DPKG_CACHE_$(1))),$(eval DPKG_CACHE_$(1) := $(shell $(2)))$(value DPKG_CACHE_$(1)),$(value DPKG_CACHE_$(1)))

DEB_VENDOR = $(call dpkg_late_eval,DEB_VENDOR,dpkg-vendor --query Vendor)
DEB_PARENT_VENDOR = $(call dpkg_late_eval,DEB_PARENT_VENDOR,dpkg-vendor --query Parent)

dpkg_vendor_derives_from_v0 = dpkg-vendor --derives-from $(1) && echo yes || echo no
dpkg_vendor_derives_from_v1 = $(shell $(dpkg_vendor_derives_from_v0))

ifeq ($(call dpkg_build_api_ge,1),yes)
dpkg_vendor_derives_from ?= $(dpkg_vendor_derives_from_v1)
else
dpkg_vendor_derives_from ?= $(dpkg_vendor_derives_from_v0)
endif

endif # dpkg_vendor_mk_included
