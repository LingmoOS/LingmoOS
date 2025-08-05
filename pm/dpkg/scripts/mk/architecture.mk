# This Makefile fragment (since dpkg 1.16.1) defines all the DEB_HOST_* and
# DEB_BUILD_* variables that dpkg-architecture can return. Existing values
# of those variables are preserved as per policy.
# All variables are exported.
#
# Note:
# - Only documented variables are considered public interfaces.
# - Expects to be included from the source tree root directory.

ifndef dpkg_architecture_mk_included
dpkg_architecture_mk_included = yes

dpkg_lazy_eval ?= $$(if $$(filter undefined,$$(flavor DPKG_CACHE_$(1))),$$(eval DPKG_CACHE_$(1) := $$(shell $(2)))$$(value DPKG_CACHE_$(1)),$$(value DPKG_CACHE_$(1)))

dpkg_architecture_setvar = export $(1) ?= $(call dpkg_lazy_eval,$(1),dpkg-architecture -q$(1))

$(foreach machine,BUILD HOST TARGET,\
  $(foreach var,ARCH ARCH_ABI ARCH_LIBC ARCH_OS ARCH_CPU ARCH_BITS ARCH_ENDIAN GNU_CPU GNU_SYSTEM GNU_TYPE MULTIARCH,\
    $(eval $(call dpkg_architecture_setvar,DEB_$(machine)_$(var)))))

endif # dpkg_architecture_mk_included
