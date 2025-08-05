# This Makefile fragment (since dpkg 1.19.0) defines the following variables
# for host tools:
#
#   AS: Assembler (since dpkg 1.19.1).
#   CPP: C preprocessor.
#   CC: C compiler.
#   CXX: C++ compiler.
#   OBJC: Objective C compiler.
#   OBJCXX: Objective C++ compiler.
#   F77: Fortran 77 compiler.
#   FC: Fortran 9x compiler.
#   LD: Linker.
#   STRIP: Strip objects (since dpkg 1.19.1).
#   OBJCOPY: Copy objects (since dpkg 1.19.1).
#   OBJDUMP: Dump objects (since dpkg 1.19.1).
#   NM: Names lister (since dpkg 1.19.1).
#   AR: Archiver (since dpkg 1.19.1).
#   RANLIB: Archive index generator (since dpkg 1.19.1).
#   PKG_CONFIG: pkg-config tool.
#   QMAKE: Qt build system generator (since dpkg 1.20.0).
#
# All the above variables have a counterpart variable for the build tool,
# as in CC → CC_FOR_BUILD.
#
# The variables are not exported by default. This can be changed by
# defining DPKG_EXPORT_BUILDTOOLS.
#
# Note:
# - Only documented variables are considered public interfaces.
# - Expects to be included from the source tree root directory.

ifndef dpkg_buildtools_mk_included
dpkg_buildtools_mk_included = yes

ifndef dpkg_datadir
  dpkg_datadir := $(dir $(lastword $(MAKEFILE_LIST)))
endif
include $(dpkg_datadir)/architecture.mk

# We set the TOOL_FOR_BUILD variables to the specified value, and the TOOL
# variables (for the host) to their triplet-prefixed form iff they are
# not defined or contain the make built-in defaults. On native builds if
# TOOL is defined and TOOL_FOR_BUILD is not, we fallback to use TOOL.
define dpkg_buildtool_setvar
  ifeq (,$(filter $(3),$(DEB_BUILD_OPTIONS)))
    ifneq (,$(filter default undefined,$(origin $(1))))
      $(1) = $(DEB_HOST_GNU_TYPE)-$(2)
    endif

    # On native build fallback to use TOOL if that's defined.
    ifeq (undefined,$(origin $(1)_FOR_BUILD))
      ifeq ($(DEB_BUILD_GNU_TYPE),$(DEB_HOST_GNU_TYPE))
        $(1)_FOR_BUILD = $$($(1))
      else
        $(1)_FOR_BUILD = $(DEB_BUILD_GNU_TYPE)-$(2)
      endif
    endif
  else
    $(1) = :
    $(1)_FOR_BUILD = :
  endif

  ifdef DPKG_EXPORT_BUILDTOOLS
    export $(1)
    export $(1)_FOR_BUILD
  endif
endef

$(eval $(call dpkg_buildtool_setvar,AS,as))
$(eval $(call dpkg_buildtool_setvar,CPP,gcc -E))
$(eval $(call dpkg_buildtool_setvar,CC,gcc))
$(eval $(call dpkg_buildtool_setvar,CXX,g++))
$(eval $(call dpkg_buildtool_setvar,OBJC,gcc))
$(eval $(call dpkg_buildtool_setvar,OBJCXX,g++))
$(eval $(call dpkg_buildtool_setvar,F77,gfortran))
$(eval $(call dpkg_buildtool_setvar,FC,gfortran))
$(eval $(call dpkg_buildtool_setvar,LD,ld))
$(eval $(call dpkg_buildtool_setvar,STRIP,strip,nostrip))
$(eval $(call dpkg_buildtool_setvar,OBJCOPY,objcopy))
$(eval $(call dpkg_buildtool_setvar,OBJDUMP,objdump))
$(eval $(call dpkg_buildtool_setvar,NM,nm))
$(eval $(call dpkg_buildtool_setvar,AR,ar))
$(eval $(call dpkg_buildtool_setvar,RANLIB,ranlib))
$(eval $(call dpkg_buildtool_setvar,PKG_CONFIG,pkgconf))
$(eval $(call dpkg_buildtool_setvar,QMAKE,qmake))

endif # dpkg_buildtools_mk_included
