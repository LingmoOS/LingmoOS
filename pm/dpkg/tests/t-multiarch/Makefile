# Copyright © 2011 Linaro Limited
# Copyright © 2011 Raphaël Hertzog <hertzog@debian.org>
# Copyright © 2011-2012 Guillem Jover <guillem@debian.org>

TESTS_DEB :=

include ../Test.mk

NATIVE_ARCH := $(shell dpkg --print-architecture)
ifeq ($(NATIVE_ARCH),i386)
	FOREIGN_ARCH := amd64
else
	FOREIGN_ARCH := i386
endif

ma-setup:
	$(DPKG) --add-architecture $(FOREIGN_ARCH)

## BUILDING THE TEST PACKAGES ##

# Params are (dirname, pkgname, ver, arch, ma, dep)
# Dirname and pkgname are auto-prefixed with "pkg-ma"
define build_pkg
PKG_TO_BUILD += pkg-ma-$(1).deb

pkg-ma-$(1):: pkg-template
	[ -e pkg-ma-$(1) ] || cp -a pkg-template pkg-ma-$(1)
	sed -i -e "s/Package: .*/Package: pkg-ma-$(2)/" \
	       -e "s/Version: .*/Version: $(3)/" \
	       -e "s/Architecture: .*/Architecture: $(4)/" \
	       -e "s/Multi-Arch: .*/Multi-Arch: $(5)/" \
	       -e "s/Depends:.*/Depends: $(6)/" \
	    pkg-ma-$(1)/DEBIAN/control
	mkdir -p pkg-ma-$(1)/m-a
	echo "In file pkg-ma-$(1)" > pkg-ma-$(1)/m-a/foo-$(1)
	touch pkg-ma-$(1)

pkg-ma-$(1).deb: pkg-ma-$(1)
	$(DPKG_BUILD_DEB) --no-check pkg-ma-$(1) pkg-ma-$(1).deb

clean-hook::
	rm -rf pkg-ma-$(1)
	rm -f pkg-ma-$(1).deb
endef

$(foreach arch,$(NATIVE_ARCH) $(FOREIGN_ARCH),\
  $(foreach ma,same foreign allowed no,\
    $(foreach version,1.0 2.0,\
      $(eval $(call build_pkg,$(ma)_$(version)_$(arch),$(ma),$(version),$(arch),$(ma),)))))

# Arch: all done separately because M-A: same can't be Arch: all
$(foreach ma,foreign allowed no,\
  $(eval $(call build_pkg,$(ma)_1.0_all,$(ma),1.0,all,$(ma),)))

$(eval $(call build_pkg,same_1.0_all,same,1.0,all,no,))

$(foreach arch,$(NATIVE_ARCH) $(FOREIGN_ARCH),\
  $(eval $(call build_pkg,self-conflict_1.0_$(arch),self-conflict,1.0,$(arch),same,))\
  $(foreach ma,same foreign foreign-any allowed allowed-any allowed-fake no,\
    $(eval $(call build_pkg,dep-on-$(ma)_1.0_$(arch),dep-on-$(ma),1.0,$(arch),foreign,pkg-ma-$(subst -,:,$(ma)) (>= 1.0)))))

$(foreach ma,same foreign foreign-any allowed allowed-any allowed-fake no,\
  $(eval $(call build_pkg,dep-on-$(ma)_1.0_all,dep-on-$(ma),1.0,all,no,pkg-ma-$(subst -,:,$(ma)) (>= 1.0))))

shared_file_types := file symlink fifo
ifdef DPKG_AS_ROOT
shared_file_types += chardev blockdev
endif
$(foreach arch,$(NATIVE_ARCH) $(FOREIGN_ARCH),\
  $(foreach version,1.0 2.0,\
    $(eval $(call build_pkg,shared-conffiles_$(version)_orig_$(arch),shared-conffiles,$(version),$(arch),same,))\
    $(eval $(call build_pkg,shared-conffiles_$(version)_alt_$(arch),shared-conffiles,$(version),$(arch),same,))\
    $(foreach variant,orig alt,\
      $(foreach type,$(shared_file_types),\
        $(eval $(call build_pkg,shared-files_$(version)_$(variant)-$(type)_$(arch),shared-files,$(version),$(arch),same,))))))

$(foreach arch,$(NATIVE_ARCH) $(FOREIGN_ARCH),\
  $(eval $(call build_pkg,triggers_1.0_$(arch),triggers,1.0,$(arch),same,))\
  $(eval $(call build_pkg,shared-diversion_1.0_$(arch),shared-diversion,1.0,$(arch),same,)))

$(eval $(call build_pkg,act-trig_1.0_all,act-trig,1.0,all,no,))
$(eval $(call build_pkg,diversion_1.0_all,diversion,1.0,all,no,))
$(eval $(call build_pkg,diverted_1.0_all,diverted,1.0,all,no,))
$(eval $(call build_pkg,db-layout_1.0_$(NATIVE_ARCH),db-layout,1.0,$(NATIVE_ARCH),no,))
$(eval $(call build_pkg,db-layout_2.0_$(NATIVE_ARCH),db-layout,2.0,$(NATIVE_ARCH),same,))

build-hook: $(PKG_TO_BUILD)

## TEST CASES BELOW ##

TEST_CASES += test-print-foreign-architecture
TEST_CASES += test-build-invalid
TEST_CASES += test-build-valid
TEST_CASES += test-install-native
TEST_CASES += test-install-foreign
TEST_CASES += test-coinstall-ma-same
TEST_CASES += test-crossgrade
TEST_CASES += test-deps
TEST_CASES += test-coinstall-shared-files
TEST_CASES += test-coinstall-shared-conffiles
TEST_CASES += test-coinstall-triggers
TEST_CASES += test-coinstall-diversions
TEST_CASES += test-dpkg-output
TEST_CASES += test-db-layout

test-case: $(TEST_CASES)

test-print-foreign-architecture:
	echo $(FOREIGN_ARCH) >ref-arch
	echo foobar >>ref-arch
	$(DPKG) --remove-architecture $(FOREIGN_ARCH)
	$(DPKG) --remove-architecture foobar
	$(call stdout_is,$(DPKG) --print-foreign-architectures,)
	$(DPKG) --add-architecture $(FOREIGN_ARCH)
	$(call stdout_is,$(DPKG) --print-foreign-architectures,$(FOREIGN_ARCH))
	$(DPKG) --add-architecture foobar
	$(DPKG) --print-foreign-architectures | cmp ref-arch -
	# Ensure all/any can't be added as foreign arch
	! $(DPKG) --add-architecture all
	! $(DPKG) --add-architecture any
	# Ensure the native arch is never considered as
	$(DPKG) --add-architecture $(NATIVE_ARCH)
	$(DPKG) --print-foreign-architectures | cmp ref-arch -
	$(DPKG) --remove-architecture foobar
	rm -f ref-arch

test-build-invalid:
	# Test build of Architecture: all with Multi-Arch: same field
	! $(DPKG_BUILD_DEB) pkg-template foo.deb
	rm -f foo.deb

test-build-valid:
	# Test build of package with valid dependency (ma-allowed:fake)
	$(DPKG_BUILD_DEB) pkg-ma-dep-on-allowed-fake_1.0_$(NATIVE_ARCH) foo.deb
	rm -f foo.deb

test-install-native:
	$(DPKG_INSTALL) pkg-ma-same_1.0_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-foreign_1.0_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-allowed_1.0_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-no_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-same:$(NATIVE_ARCH))
	$(call pkg_is_installed,pkg-ma-foreign)
	$(call pkg_is_installed,pkg-ma-allowed)
	$(call pkg_is_installed,pkg-ma-no)
	$(DPKG_PURGE) pkg-ma-same:$(NATIVE_ARCH) pkg-ma-foreign pkg-ma-allowed pkg-ma-no
	$(call pkg_is_not_installed,pkg-ma-same:$(NATIVE_ARCH))
	$(call pkg_is_not_installed,pkg-ma-foreign)
	$(call pkg_is_not_installed,pkg-ma-allowed)
	$(call pkg_is_not_installed,pkg-ma-no)

test-install-foreign:
	$(DPKG) --remove-architecture $(FOREIGN_ARCH)
	# Installation of foreign arch packages must fail by default
	! $(DPKG_INSTALL) pkg-ma-same_1.0_$(FOREIGN_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-foreign_1.0_$(FOREIGN_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-allowed_1.0_$(FOREIGN_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-no_1.0_$(FOREIGN_ARCH).deb
	# Once architecture has been added it should succeed
	$(DPKG) --add-architecture $(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-same_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-foreign_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-allowed_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-no_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_installed,pkg-ma-same:$(FOREIGN_ARCH))
	$(call pkg_is_installed,pkg-ma-foreign:$(FOREIGN_ARCH))
	$(call pkg_is_installed,pkg-ma-allowed:$(FOREIGN_ARCH))
	$(call pkg_is_installed,pkg-ma-no:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-same:$(FOREIGN_ARCH) pkg-ma-foreign:$(FOREIGN_ARCH) pkg-ma-allowed:$(FOREIGN_ARCH) pkg-ma-no:$(FOREIGN_ARCH)
	$(call pkg_is_not_installed,pkg-ma-same:$(FOREIGN_ARCH))
	$(call pkg_is_not_installed,pkg-ma-foreign:$(FOREIGN_ARCH))
	$(call pkg_is_not_installed,pkg-ma-allowed:$(FOREIGN_ARCH))
	$(call pkg_is_not_installed,pkg-ma-no:$(FOREIGN_ARCH))

test-coinstall-ma-same: ma-setup
	# Coinstall 2 Multi-Arch: same packages
	$(DPKG_INSTALL) pkg-ma-same_1.0_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-same_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_installed,pkg-ma-same:$(NATIVE_ARCH))
	$(call pkg_is_installed,pkg-ma-same:$(FOREIGN_ARCH))
	# Upgrade one, the other must be unconfigured
	$(DPKG_UNPACK) pkg-ma-same_2.0_$(NATIVE_ARCH).deb
	$(call pkg_status_is,pkg-ma-same:$(NATIVE_ARCH),install ok unpacked)
	$(call pkg_status_is,pkg-ma-same:$(FOREIGN_ARCH),install ok half-configured)
	# Configure one alone must fail
	! $(DPKG_CONFIGURE) pkg-ma-same:$(NATIVE_ARCH)
	! $(DPKG_CONFIGURE) pkg-ma-same:$(FOREIGN_ARCH)
	# Upgrade the other and it's now possible to configure them
	$(DPKG_UNPACK) pkg-ma-same_2.0_$(FOREIGN_ARCH).deb
	$(DPKG_CONFIGURE) --pending
	$(call pkg_is_installed,pkg-ma-same:$(NATIVE_ARCH))
	$(call pkg_is_installed,pkg-ma-same:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-same:$(NATIVE_ARCH) pkg-ma-same:$(FOREIGN_ARCH)

TEST_CASES_CROSSGRADE :=
TEST_CASES_CROSSGRADE += test-crossgrade-foreign-allowed

test-crossgrade: $(TEST_CASES_CROSSGRADE)

test-crossgrade-foreign-allowed: ma-setup
	# Ensure we can cross-grade Multi-Arch: foreign/allowed
	$(DPKG_INSTALL) pkg-ma-foreign_1.0_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-foreign_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-allowed_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-allowed_1.0_$(NATIVE_ARCH).deb
	$(DPKG_PURGE) pkg-ma-foreign:$(NATIVE_ARCH) pkg-ma-allowed:$(FOREIGN_ARCH)

TEST_CASES_CROSSGRADE :=
TEST_CASES_CROSSGRADE += test-crossgrade-any-all-same
TEST_CASES_CROSSGRADE += test-crossgrade-any-all-foreign
TEST_CASES_CROSSGRADE += test-crossgrade-any-all-allowed
TEST_CASES_CROSSGRADE += test-crossgrade-all-any-same
TEST_CASES_CROSSGRADE += test-crossgrade-all-any-foreign
TEST_CASES_CROSSGRADE += test-crossgrade-all-any-allowed
TEST_CASES_CROSSGRADE += test-crossgrade-same-all
TEST_CASES_CROSSGRADE += test-crossgrade-no

test-crossgrade: $(TEST_CASES_CROSSGRADE)

test-crossgrade-any-all-%: ma-setup
	# M-A: $* / Upgrade from arch: any-native -> all
	$(DPKG_INSTALL) pkg-ma-$*_1.0_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-$*_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-$*:all)
	$(call pkg_is_not_installed,pkg-ma-$*:$(NATIVE_ARCH))
	$(call pkg_field_is,pkg-ma-$*,Architecture,all)
	$(DPKG_PURGE) pkg-ma-$*
	$(call pkg_is_not_installed,pkg-ma-$*)
	# M-A: $* / Upgrade from arch: any-foreign -> all
	$(DPKG_INSTALL) pkg-ma-$*_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-$*_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-$*:all)
	$(call pkg_is_not_installed,pkg-ma-$*:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-$*

test-crossgrade-all-any-%: ma-setup
	# M-A: $* / Upgrade from arch: all -> any-native
	$(DPKG_INSTALL) pkg-ma-$*_1.0_all.deb
	$(DPKG_INSTALL) pkg-ma-$*_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-$*:$(NATIVE_ARCH))
	$(call pkg_is_not_installed,pkg-ma-$*:all)
	$(call pkg_field_is,pkg-ma-$*,Architecture,$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-$*:$(NATIVE_ARCH)
	$(call pkg_is_not_installed,pkg-ma-$*:all)
	$(call pkg_is_not_installed,pkg-ma-$*:$(NATIVE_ARCH))
	# M-A: $* / Upgrade from arch: all -> any-foreign
	$(DPKG_INSTALL) pkg-ma-$*_1.0_all.deb
	$(DPKG_INSTALL) pkg-ma-$*_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_not_installed,pkg-ma-$*:all)
	$(call pkg_is_installed,pkg-ma-$*:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-$*

test-crossgrade-same-all: ma-setup
	# Several M-A: same installed, can't upgrade to single arch all
	$(DPKG_INSTALL) pkg-ma-same_1.0_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-same_1.0_$(FOREIGN_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-same_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-same:$(NATIVE_ARCH))
	$(call pkg_is_installed,pkg-ma-same:$(FOREIGN_ARCH))
	$(call pkg_is_not_installed,pkg-ma-same:all)
	$(DPKG_PURGE) pkg-ma-same:$(NATIVE_ARCH) pkg-ma-same:$(FOREIGN_ARCH)

pkg-ma-no_1.0_$(NATIVE_ARCH) pkg-ma-no_1.0_$(FOREIGN_ARCH):: pkg-template
	echo "A conffile" > $@/m-a/conffile-$@
	echo "/m-a/conffile-$@" > $@/DEBIAN/conffiles

test-crossgrade-no: ma-setup
	$(DPKG_INSTALL) pkg-ma-no_1.0_$(NATIVE_ARCH).deb
	$(DPKG_REMOVE) pkg-ma-no:$(NATIVE_ARCH)
	$(call pkg_status_is,pkg-ma-no:$(NATIVE_ARCH),deinstall ok config-files)
	$(call pkg_is_not_installed,pkg-ma-no:$(FOREIGN_ARCH))
	$(call stdout_has,$(DPKG_QUERY) -L pkg-ma-no,/m-a/conffile-pkg-ma-no_1.0_$(NATIVE_ARCH))
	$(DPKG_INSTALL) pkg-ma-no_1.0_$(FOREIGN_ARCH).deb
	# Verify the replaced package disappeared
	$(call pkg_is_not_installed,pkg-ma-no:$(NATIVE_ARCH))
	$(call pkg_is_installed,pkg-ma-no:$(FOREIGN_ARCH))
	$(call stdout_has,$(DPKG_QUERY) -L pkg-ma-no:$(FOREIGN_ARCH),/m-a/conffile-pkg-ma-no_1.0_$(FOREIGN_ARCH))
	# Ensure the conffile of the disappeared package is still tracked
	$(call stdout_has,$(DPKG_QUERY) -L pkg-ma-no:$(FOREIGN_ARCH),/m-a/conffile-pkg-ma-no_1.0_$(NATIVE_ARCH))
	$(call stdout_has,$(DPKG_QUERY) -f '$${Conffiles}' -W pkg-ma-no:$(FOREIGN_ARCH), /m-a/conffile-pkg-ma-no_1.0_$(NATIVE_ARCH) [a-f0-9]+ obsolete$$)
	$(DPKG_PURGE) pkg-ma-no:$(FOREIGN_ARCH)

TEST_CASES_DEPS :=
TEST_CASES_DEPS += test-deps-same
TEST_CASES_DEPS += test-deps-foreign
TEST_CASES_DEPS += test-deps-allowed
TEST_CASES_DEPS += test-deps-no
TEST_CASES_DEPS += test-deps-self-conflict
TEST_CASES_DEPS += test-deps-force

test-deps: $(TEST_CASES_DEPS)

test-deps-same: ma-setup
	# Native M-A: same is installed, no cross-arch dep allowed
	$(DPKG_INSTALL) pkg-ma-same_1.0_$(NATIVE_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-dep-on-same_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-dep-on-same:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-same_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-dep-on-same)
	$(DPKG_PURGE) pkg-ma-dep-on-same
	$(DPKG_INSTALL) pkg-ma-dep-on-same_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-same:$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-same:$(NATIVE_ARCH)
	$(DPKG_PURGE) pkg-ma-same:$(NATIVE_ARCH)
	# Foreign M-A: same is installed
	$(DPKG_INSTALL) pkg-ma-same_1.0_$(FOREIGN_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-dep-on-same_1.0_$(NATIVE_ARCH).deb
	$(DPKG_PURGE) pkg-ma-dep-on-same:$(NATIVE_ARCH)
	! $(DPKG_INSTALL) pkg-ma-dep-on-same_1.0_all.deb
	$(DPKG_PURGE) pkg-ma-dep-on-same
	$(DPKG_INSTALL) pkg-ma-dep-on-same_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-same:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-same:$(FOREIGN_ARCH)
	$(DPKG_PURGE) pkg-ma-same:$(FOREIGN_ARCH)
	# Arch: all M-A: same can't exist

test-deps-foreign: ma-setup
	# Native M-A: foreign is installed
	$(DPKG_INSTALL) pkg-ma-foreign_1.0_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-dep-on-foreign_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-foreign:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-foreign:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-foreign_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-dep-on-foreign)
	$(DPKG_PURGE) pkg-ma-dep-on-foreign
	$(DPKG_INSTALL) pkg-ma-dep-on-foreign_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-foreign:$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-foreign:$(NATIVE_ARCH)
	$(DPKG_PURGE) pkg-ma-foreign:$(NATIVE_ARCH)
	# Foreign M-A: foreign is installed
	$(DPKG_INSTALL) pkg-ma-foreign_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-dep-on-foreign_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-foreign:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-foreign:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-foreign_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-dep-on-foreign)
	$(DPKG_PURGE) pkg-ma-dep-on-foreign
	$(DPKG_INSTALL) pkg-ma-dep-on-foreign_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-foreign:$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-foreign:$(NATIVE_ARCH)
	$(DPKG_PURGE) pkg-ma-foreign:$(FOREIGN_ARCH)
	# Arch: all M-A: foreign is installed
	$(DPKG_INSTALL) pkg-ma-foreign_1.0_all.deb
	$(DPKG_INSTALL) pkg-ma-dep-on-foreign_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-foreign:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-foreign:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-foreign_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-dep-on-foreign)
	$(DPKG_PURGE) pkg-ma-dep-on-foreign
	$(DPKG_INSTALL) pkg-ma-dep-on-foreign_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-foreign:$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-foreign:$(NATIVE_ARCH)
	$(DPKG_PURGE) pkg-ma-foreign

test-deps-allowed: ma-setup
	## Native M-A: allowed is installed
	$(DPKG_INSTALL) pkg-ma-allowed_1.0_$(NATIVE_ARCH).deb
	# Non-annotated dependency on pkg-ma-allowed
	! $(DPKG_INSTALL) pkg-ma-dep-on-allowed_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-dep-on-allowed:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed:$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-allowed:$(NATIVE_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed)
	$(DPKG_PURGE) pkg-ma-dep-on-allowed
	# Annotated dep: pkg-ma-allowed:any
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed-any_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed-any:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-allowed-any:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed-any_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed-any:$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-allowed-any:$(NATIVE_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed-any_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed-any)
	$(DPKG_PURGE) pkg-ma-dep-on-allowed-any
	$(DPKG_PURGE) pkg-ma-allowed:$(NATIVE_ARCH)
	## Foreign M-A: allowed is installed
	$(DPKG_INSTALL) pkg-ma-allowed_1.0_$(FOREIGN_ARCH).deb
	# Non-annotated dependency on pkg-ma-allowed
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-allowed:$(FOREIGN_ARCH)
	! $(DPKG_INSTALL) pkg-ma-dep-on-allowed_1.0_$(NATIVE_ARCH).deb
	$(DPKG_PURGE) pkg-ma-dep-on-allowed:$(NATIVE_ARCH)
	! $(DPKG_INSTALL) pkg-ma-dep-on-allowed_1.0_all.deb
	$(DPKG_PURGE) pkg-ma-dep-on-allowed
	# Annotated dep: pkg-ma-allowed:any
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed-any_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed-any:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-allowed-any:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed-any_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed-any:$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-allowed-any:$(NATIVE_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed-any_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed-any)
	$(DPKG_PURGE) pkg-ma-dep-on-allowed-any
	$(DPKG_PURGE) pkg-ma-allowed:$(FOREIGN_ARCH)
	## Arch: all M-A: allowed is installed
	$(DPKG_INSTALL) pkg-ma-allowed_1.0_all.deb
	# Non-annotated dependency on pkg-ma-allowed
	! $(DPKG_INSTALL) pkg-ma-dep-on-allowed_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-dep-on-allowed:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed:$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-allowed:$(NATIVE_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed)
	$(DPKG_PURGE) pkg-ma-dep-on-allowed
	# Annotated dep: pkg-ma-allowed:any
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed-any_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed-any:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-allowed-any:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed-any_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed-any:$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-allowed-any:$(NATIVE_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-allowed-any_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-dep-on-allowed-any)
	$(DPKG_PURGE) pkg-ma-dep-on-allowed-any
	$(DPKG_PURGE) pkg-ma-allowed

test-deps-no: ma-setup
	# Native M-A: no is installed
	$(DPKG_INSTALL) pkg-ma-no_1.0_$(NATIVE_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-dep-on-no_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-dep-on-no:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-no_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-dep-on-no)
	$(DPKG_PURGE) pkg-ma-dep-on-no
	$(DPKG_INSTALL) pkg-ma-dep-on-no_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-no:$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-no:$(NATIVE_ARCH)
	$(DPKG_PURGE) pkg-ma-no:$(NATIVE_ARCH)
	# Foreign M-A: no is installed (with --force-architecture)
	$(DPKG_INSTALL) --force-architecture pkg-ma-no_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-dep-on-no_1.0_$(FOREIGN_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-no:$(FOREIGN_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-no:$(FOREIGN_ARCH)
	! $(DPKG_INSTALL) pkg-ma-dep-on-no_1.0_all.deb
	$(DPKG_PURGE) pkg-ma-dep-on-no
	! $(DPKG_INSTALL) pkg-ma-dep-on-no_1.0_$(NATIVE_ARCH).deb
	$(DPKG_PURGE) pkg-ma-dep-on-no:$(NATIVE_ARCH)
	$(DPKG_PURGE) pkg-ma-no:$(FOREIGN_ARCH)
	# Arch: all M-A: no is installed
	$(DPKG_INSTALL) pkg-ma-no_1.0_all.deb
	! $(DPKG_INSTALL) pkg-ma-dep-on-no_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-dep-on-no:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-dep-on-no_1.0_all.deb
	$(call pkg_is_installed,pkg-ma-dep-on-no)
	$(DPKG_PURGE) pkg-ma-dep-on-no
	$(DPKG_INSTALL) pkg-ma-dep-on-no_1.0_$(NATIVE_ARCH).deb
	$(call pkg_is_installed,pkg-ma-dep-on-no:$(NATIVE_ARCH))
	$(DPKG_PURGE) pkg-ma-dep-on-no:$(NATIVE_ARCH)
	$(DPKG_PURGE) pkg-ma-no

pkg-ma-self-conflict_1.0_$(NATIVE_ARCH) pkg-ma-self-conflict_1.0_$(FOREIGN_ARCH):: pkg-template
	grep -q "Conflicts: " $@/DEBIAN/control || \
	(echo "Conflicts: virtual-foo" >> $@/DEBIAN/control; \
	echo "Provides: virtual-foo" >> $@/DEBIAN/control)

test-deps-self-conflict: ma-setup
	$(DPKG_INSTALL) pkg-ma-self-conflict_1.0_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-self-conflict_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-self-conflict:$(NATIVE_ARCH)
	$(DPKG_PURGE) pkg-ma-self-conflict:$(FOREIGN_ARCH)

test-deps-force: ma-setup
	# Try --force-depends when dependency is missing
	$(DPKG_INSTALL) --force-depends pkg-ma-dep-on-no_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-dep-on-no:$(FOREIGN_ARCH)
	# Try --force-depends when dependency is of the wrong arch
	$(DPKG_INSTALL) pkg-ma-no_1.0_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) --force-depends pkg-ma-dep-on-no_1.0_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-dep-on-no:$(FOREIGN_ARCH)
	$(DPKG_PURGE) pkg-ma-no

common_file = /m-a/common-file

# Further customize test packages for test-coinstall-shared-files
define customize_shared_files
pkg-ma-shared-files_$(1)_$(2)-file_$(3):: pkg-template
	echo "Variant: $(2) Version: $(1)" > pkg-ma-shared-files_$(1)_$(2)-file_$(3)$(common_file)

pkg-ma-shared-files_$(1)_$(2)-symlink_$(3):: pkg-template
	ln -sf "$(2)-$(1)" pkg-ma-shared-files_$(1)_$(2)-symlink_$(3)$(common_file)

pkg-ma-shared-files_$(1)_$(2)-fifo_$(3):: pkg-template
	test -e pkg-ma-shared-files_$(1)_$(2)-fifo_$(3)$(common_file) || \
	$(BEROOT) mkfifo pkg-ma-shared-files_$(1)_$(2)-fifo_$(3)$(common_file)

pkg-ma-shared-files_$(1)_$(2)-chardev_$(3):: pkg-template
	[ "$(2)" = "orig" ] && major=1 || major=2; \
	[ "$(1)" = "1.0" ] && minor=3 || minor=4; \
	test -e pkg-ma-shared-files_$(1)_$(2)-chardev_$(3)$(common_file) || \
	$(BEROOT) mknod pkg-ma-shared-files_$(1)_$(2)-chardev_$(3)$(common_file) c $$$$major $$$$minor

pkg-ma-shared-files_$(1)_$(2)-blockdev_$(3):: pkg-template
	[ "$(2)" = "orig" ] && major=1 || major=2; \
	[ "$(1)" = "1.0" ] && minor=3 || minor=4; \
	test -e pkg-ma-shared-files_$(1)_$(2)-blockdev_$(3)$(common_file) || \
	$(BEROOT) mknod pkg-ma-shared-files_$(1)_$(2)-blockdev_$(3)$(common_file) b $$$$major $$$$minor
endef

$(foreach version,1.0 2.0,$(foreach variant,orig alt,\
  $(eval $(call customize_shared_files,$(version),$(variant),$(NATIVE_ARCH)))\
  $(eval $(call customize_shared_files,$(version),$(variant),$(FOREIGN_ARCH)))))

test-coinstall-shared-files: ma-setup
	# Ensure we can't install a shared file conflicting with a
	# multi-arch sibling package of the same version
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-file_$(NATIVE_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-shared-files_1.0_alt-symlink_$(FOREIGN_ARCH).deb
	test -f "$(DPKG_INSTDIR)$(common_file)"
	! $(DPKG_INSTALL) pkg-ma-shared-files_1.0_alt-fifo_$(FOREIGN_ARCH).deb
	test -f "$(DPKG_INSTDIR)$(common_file)"
ifdef DPKG_AS_ROOT
	! $(DPKG_INSTALL) pkg-ma-shared-files_1.0_alt-chardev_$(FOREIGN_ARCH).deb
	test -f "$(DPKG_INSTDIR)$(common_file)"
	! $(DPKG_INSTALL) pkg-ma-shared-files_1.0_alt-blockdev_$(FOREIGN_ARCH).deb
	test -f "$(DPKG_INSTDIR)$(common_file)"
endif
	! $(DPKG_INSTALL) pkg-ma-shared-files_1.0_alt-file_$(FOREIGN_ARCH).deb
	grep -q "Variant: orig Version: 1.0" "$(DPKG_INSTDIR)$(common_file)"
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-file_$(FOREIGN_ARCH).deb
	$(call stdout_has,$(DPKG_QUERY) -L pkg-ma-shared-files:$(NATIVE_ARCH),$(common_file))
	$(call stdout_has,$(DPKG_QUERY) -L pkg-ma-shared-files:$(FOREIGN_ARCH),$(common_file))
	! $(DPKG_INSTALL) pkg-ma-shared-files_1.0_alt-file_$(FOREIGN_ARCH).deb
	# But that we can install another version while upgrading
	$(DPKG_UNPACK) pkg-ma-shared-files_2.0_orig-file_$(NATIVE_ARCH).deb
	grep -q "Variant: orig Version: 2.0" "$(DPKG_INSTDIR)$(common_file)"
	! $(DPKG_UNPACK) pkg-ma-shared-files_2.0_alt-file_$(FOREIGN_ARCH).deb
	$(DPKG_UNPACK) pkg-ma-shared-files_2.0_orig-file_$(FOREIGN_ARCH).deb
	$(DPKG_CONFIGURE) --pending
	$(DPKG_UNPACK) pkg-ma-shared-files_1.0_alt-file_$(NATIVE_ARCH).deb
	grep -q "Variant: alt" "$(DPKG_INSTDIR)$(common_file)"
	# Even during upgrade you can't come with a conflicting version
	! $(DPKG_UNPACK) pkg-ma-shared-files_1.0_orig-file_$(FOREIGN_ARCH).deb
	$(DPKG_UNPACK) pkg-ma-shared-files_1.0_alt-file_$(FOREIGN_ARCH).deb
	$(DPKG_CONFIGURE) --pending
	$(DPKG_PURGE) pkg-ma-shared-files:$(NATIVE_ARCH)
	$(DPKG_PURGE) pkg-ma-shared-files:$(FOREIGN_ARCH)
	# Ensure we can re-install a conflicting version of a package if
	# no multi-arch siblings are installed
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-file_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_alt-file_$(NATIVE_ARCH).deb
	$(DPKG_PURGE) pkg-ma-shared-files:$(NATIVE_ARCH)
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-file_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_alt-file_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-shared-files:$(FOREIGN_ARCH)
	# Ensure files are kept during removal
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-file_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-file_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-shared-files:$(FOREIGN_ARCH)
	grep -q "Variant: orig Version: 1.0" "$(DPKG_INSTDIR)$(common_file)"
	$(call stdout_has,$(DPKG_QUERY) -L pkg-ma-shared-files:$(NATIVE_ARCH),$(common_file))
	$(DPKG_PURGE) pkg-ma-shared-files:$(NATIVE_ARCH)
	# Ensure we detect differences with non-files too
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-symlink_$(NATIVE_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-shared-files_1.0_alt-symlink_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-symlink_$(FOREIGN_ARCH).deb
	$(call stdout_is,readlink "$(DPKG_INSTDIR)$(common_file)",orig-1.0)
	$(DPKG_PURGE) pkg-ma-shared-files:$(NATIVE_ARCH) pkg-ma-shared-files:$(FOREIGN_ARCH)
ifdef DPKG_AS_ROOT
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-chardev_$(NATIVE_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-shared-files_1.0_alt-chardev_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-chardev_$(FOREIGN_ARCH).deb
	test -c "$(DPKG_INSTDIR)$(common_file)"
	$(call stdout_is,stat -c "%t %T" "$(DPKG_INSTDIR)$(common_file)",1 3)
	$(DPKG_PURGE) pkg-ma-shared-files:$(NATIVE_ARCH) pkg-ma-shared-files:$(FOREIGN_ARCH)
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-blockdev_$(NATIVE_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-shared-files_1.0_alt-blockdev_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-blockdev_$(FOREIGN_ARCH).deb
	test -b "$(DPKG_INSTDIR)$(common_file)"
	$(call stdout_is,stat -c "%t %T" "$(DPKG_INSTDIR)$(common_file)",1 3)
	$(DPKG_PURGE) pkg-ma-shared-files:$(NATIVE_ARCH) pkg-ma-shared-files:$(FOREIGN_ARCH)
endif

# Further customize test packages for test-coinstall-shared-conffiles
define customize_shared_conffiles
pkg-ma-shared-conffiles_$(1)_$(2)_$(3):: pkg-template
	echo "Variant: $(2) Version: $(1)" > pkg-ma-shared-conffiles_$(1)_$(2)_$(3)/m-a/conffile
	echo "/m-a/conffile" > pkg-ma-shared-conffiles_$(1)_$(2)_$(3)/DEBIAN/conffiles
endef

$(foreach version,1.0 2.0,$(foreach variant,orig alt,\
  $(eval $(call customize_shared_conffiles,$(version),$(variant),$(NATIVE_ARCH)))\
  $(eval $(call customize_shared_conffiles,$(version),$(variant),$(FOREIGN_ARCH)))))

test-coinstall-shared-conffiles: ma-setup
	# Ensure we can't install a shared conffile conflicting with a
	# multi-arch sibling package of the same version
	$(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_orig_$(NATIVE_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_alt_$(FOREIGN_ARCH).deb
	grep -q "Variant: orig Version: 1.0" "$(DPKG_INSTDIR)/m-a/conffile"
	$(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_orig_$(FOREIGN_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_alt_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-shared-conffiles:$(NATIVE_ARCH)
	$(DPKG_PURGE) pkg-ma-shared-conffiles:$(FOREIGN_ARCH)
	# Ensure this is still true even if the installed conffile is modified
	$(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_orig_$(NATIVE_ARCH).deb
	$(BEROOT) sh -c "echo 'MODIFIED' >>'$(DPKG_INSTDIR)/m-a/conffile'"
	! $(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_alt_$(FOREIGN_ARCH).deb
	grep -q "Variant: orig Version: 1.0" "$(DPKG_INSTDIR)/m-a/conffile"
	$(DPKG_INSTALL) --force-confold pkg-ma-shared-conffiles_1.0_orig_$(FOREIGN_ARCH).deb
	! $(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_alt_$(FOREIGN_ARCH).deb
	# Ensure we can install another version while upgrading even
	# with a modified conffile
	$(DPKG_UNPACK) pkg-ma-shared-conffiles_2.0_orig_$(NATIVE_ARCH).deb
	grep -q "Variant: orig Version: 2.0" "$(DPKG_INSTDIR)/m-a/conffile.dpkg-new"
	! $(DPKG_UNPACK) pkg-ma-shared-conffiles_2.0_alt_$(FOREIGN_ARCH).deb
	grep -q "Variant: orig Version: 2.0" "$(DPKG_INSTDIR)/m-a/conffile.dpkg-new"
	$(DPKG_UNPACK) pkg-ma-shared-conffiles_2.0_orig_$(FOREIGN_ARCH).deb
	$(DPKG_CONFIGURE) --force-confnew --pending
	grep -q "Variant: orig Version: 2.0" "$(DPKG_INSTDIR)/m-a/conffile"
	# Ensure the conffile hashes are correct, i.e. the new hashes have
	# propagated to all instances
	$(call stdout_is,$(DPKG_QUERY) -f '$${Conffiles}' -W pkg-ma-shared-conffiles:$(NATIVE_ARCH), /m-a/conffile 7404b80581282025ee3ec6689e111114)
	$(call stdout_is,$(DPKG_QUERY) -f '$${Conffiles}' -W pkg-ma-shared-conffiles:$(FOREIGN_ARCH), /m-a/conffile 7404b80581282025ee3ec6689e111114)
	# Ensure we can reinstall coinstallable packages with conffiles on
	# the same run
	$(DPKG_INSTALL) pkg-ma-shared-conffiles_2.0_orig_$(NATIVE_ARCH).deb \
	                pkg-ma-shared-conffiles_2.0_orig_$(FOREIGN_ARCH).deb
	grep -q "Variant: orig Version: 2.0" "$(DPKG_INSTDIR)/m-a/conffile"
	# Ensure we can reinstall coinstallable packages with conffiles on
	# different runs
	$(DPKG_UNPACK) pkg-ma-shared-conffiles_2.0_orig_$(FOREIGN_ARCH).deb
	$(DPKG_UNPACK) pkg-ma-shared-conffiles_2.0_orig_$(NATIVE_ARCH).deb
	$(DPKG_CONFIGURE) pkg-ma-shared-conffiles:$(NATIVE_ARCH)
	$(DPKG_CONFIGURE) pkg-ma-shared-conffiles:$(FOREIGN_ARCH)
	grep -q "Variant: orig Version: 2.0" "$(DPKG_INSTDIR)/m-a/conffile"
	# Another time with a downgrade and an unmodified conffile
	$(DPKG_UNPACK) pkg-ma-shared-conffiles_1.0_alt_$(NATIVE_ARCH).deb
	grep -q "Variant: alt" "$(DPKG_INSTDIR)/m-a/conffile.dpkg-new"
	! $(DPKG_UNPACK) pkg-ma-shared-conffiles_1.0_orig_$(FOREIGN_ARCH).deb
	$(DPKG_UNPACK) pkg-ma-shared-conffiles_1.0_alt_$(FOREIGN_ARCH).deb
	$(DPKG_CONFIGURE) --force-confnew --pending
	# Verify shared conffiles are forgotten on remove, and really
	# removed on purge of last package
	$(DPKG_REMOVE) pkg-ma-shared-conffiles:$(NATIVE_ARCH)
	! $(call stdout_has,$(DPKG_QUERY) -L pkg-ma-shared-conffiles:$(NATIVE_ARCH),/m-a/conffile)
	$(DPKG_PURGE) pkg-ma-shared-conffiles:$(NATIVE_ARCH)
	test -e "$(DPKG_INSTDIR)/m-a/conffile"
	$(DPKG_PURGE) pkg-ma-shared-conffiles:$(FOREIGN_ARCH)
	! test -e "$(DPKG_INSTDIR)/m-a/conffile"
	# Ensure we can re-install a conflicting version of a package if
	# no multi-arch siblings are installed
	$(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_orig_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_alt_$(NATIVE_ARCH).deb
	$(DPKG_PURGE) pkg-ma-shared-conffiles:$(NATIVE_ARCH)
	$(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_orig_$(FOREIGN_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_alt_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-shared-conffiles:$(FOREIGN_ARCH)
	# Same with modified conffile
	$(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_orig_$(NATIVE_ARCH).deb
	$(BEROOT) sh -c "echo 'MODIFIED' >>'$(DPKG_INSTDIR)/m-a/conffile'"
	$(DPKG_INSTALL) --force-confnew pkg-ma-shared-conffiles_1.0_alt_$(NATIVE_ARCH).deb
	$(DPKG_PURGE) pkg-ma-shared-conffiles:$(NATIVE_ARCH)
	$(DPKG_INSTALL) pkg-ma-shared-conffiles_1.0_orig_$(FOREIGN_ARCH).deb
	$(BEROOT) sh -c "echo 'MODIFIED' >>'$(DPKG_INSTDIR)/m-a/conffile'"
	$(DPKG_INSTALL) --force-confnew pkg-ma-shared-conffiles_1.0_alt_$(FOREIGN_ARCH).deb
	$(DPKG_PURGE) pkg-ma-shared-conffiles:$(FOREIGN_ARCH)

# Further customize test packages for test-coinstall-triggers
pkg-ma-triggers_1.0_$(NATIVE_ARCH) pkg-ma-triggers_1.0_$(FOREIGN_ARCH):: pkg-template
	echo "#!/bin/sh" >$@/DEBIAN/postinst
	echo '[ "$$1" = "triggered" ] || exit 0' >>$@/DEBIAN/postinst
	echo 'case " $$2 " in' >>$@/DEBIAN/postinst
	echo '*" /m-a/trigger-watch "*) ' >>$@/DEBIAN/postinst
	echo '   echo "FILE $$DPKG_MAINTSCRIPT_ARCH" >>"$$DPKG_ROOT/m-a/triggers" ;;' >>$@/DEBIAN/postinst
	echo 'esac' >>$@/DEBIAN/postinst
	echo 'case " $$2 " in' >>$@/DEBIAN/postinst
	echo '*" test-m-a-same "*)' >>$@/DEBIAN/postinst
	echo '   echo "MANUAL $$DPKG_MAINTSCRIPT_ARCH" >>"$$DPKG_ROOT/m-a/triggers" ;;' >>$@/DEBIAN/postinst
	echo 'esac' >>$@/DEBIAN/postinst
	chmod 755 $@/DEBIAN/postinst
	echo 'interest /m-a/trigger-watch' >$@/DEBIAN/triggers
	echo 'interest test-m-a-same' >>$@/DEBIAN/triggers

pkg-ma-act-trig_1.0_all:: pkg-template
	echo "Activates a trigger" >$@/m-a/trigger-watch
	echo "#!/bin/sh" >$@/DEBIAN/postinst
	echo "dpkg-trigger test-m-a-same" >>$@/DEBIAN/postinst
	chmod 755 $@/DEBIAN/postinst

test-coinstall-triggers: ma-setup
	-cat "$(DPKG_ADMINDIR)/triggers/File"
	$(DPKG_INSTALL) pkg-ma-triggers_1.0_$(NATIVE_ARCH).deb
	cat "$(DPKG_ADMINDIR)/triggers/File"
	$(DPKG_INSTALL) pkg-ma-triggers_1.0_$(FOREIGN_ARCH).deb
	cat "$(DPKG_ADMINDIR)/triggers/File"
	# Test file trigger processing
	$(BEROOT) sh -c ":>'$(DPKG_INSTDIR)/m-a/triggers'"
	$(DPKG_UNPACK) --no-triggers pkg-ma-act-trig_1.0_all.deb
	$(call pkg_status_is,pkg-ma-triggers:$(NATIVE_ARCH),install ok triggers-pending)
	$(call pkg_status_is,pkg-ma-triggers:$(FOREIGN_ARCH),install ok triggers-pending)
	$(DPKG_CONFIGURE) pkg-ma-triggers:$(NATIVE_ARCH) pkg-ma-triggers:$(FOREIGN_ARCH)
	$(call pkg_is_installed,pkg-ma-triggers:$(NATIVE_ARCH))
	$(call pkg_is_installed,pkg-ma-triggers:$(FOREIGN_ARCH))
	$(call stdout_is,grep -c FILE "$(DPKG_INSTDIR)/m-a/triggers",2)
	grep -q "FILE $(NATIVE_ARCH)" "$(DPKG_INSTDIR)/m-a/triggers"
	grep -q "FILE $(FOREIGN_ARCH)" "$(DPKG_INSTDIR)/m-a/triggers"
	# Test manual trigger processing
	$(DPKG_CONFIGURE) pkg-ma-act-trig
	$(call pkg_is_installed,pkg-ma-triggers:$(NATIVE_ARCH))
	$(call pkg_is_installed,pkg-ma-triggers:$(FOREIGN_ARCH))
	$(call stdout_is,grep -c MANUAL "$(DPKG_INSTDIR)/m-a/triggers",2)
	grep -q "MANUAL $(NATIVE_ARCH)" "$(DPKG_INSTDIR)/m-a/triggers"
	grep -q "MANUAL $(FOREIGN_ARCH)" "$(DPKG_INSTDIR)/m-a/triggers"
	# Clean up
	$(DPKG_PURGE) pkg-ma-act-trig
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/m-a/triggers"
	$(DPKG_PURGE) pkg-ma-triggers:$(NATIVE_ARCH)
	$(DPKG_PURGE) pkg-ma-triggers:$(FOREIGN_ARCH)

# Further customize test packages for test-coinstall-diversions
pkg-ma-diversion_1.0_all:: pkg-template
	echo "File provided by pkg-ma-diversion" >$@$(common_file)
	echo "#!/bin/sh" >$@/DEBIAN/preinst
	echo '[ "$$1" = "install" ] || exit 0' >>$@/DEBIAN/preinst
	echo 'dpkg-divert --add --rename $(common_file)' >>$@/DEBIAN/preinst
	echo "#!/bin/sh" >$@/DEBIAN/postrm
	echo '[ "$$1" = "remove" ] || exit 0' >>$@/DEBIAN/postrm
	echo 'dpkg-divert --remove --rename $(common_file)' >>$@/DEBIAN/postrm
	chmod 755 $@/DEBIAN/preinst $@/DEBIAN/postrm

pkg-ma-diverted_1.0_all:: pkg-template
	echo "File provided by pkg-ma-diverted" >$@$(common_file)

pkg-ma-shared-diversion_1.0_$(NATIVE_ARCH) pkg-ma-shared-diversion_1.0_$(FOREIGN_ARCH):: pkg-template
	echo "File provided by pkg-ma-shared-diversion" >$@$(common_file)
	echo "#!/bin/sh" >$@/DEBIAN/preinst
	echo '[ "$$1" = "install" ] || exit 0' >>$@/DEBIAN/preinst
	echo 'dpkg-divert --add --rename $(common_file)' >>$@/DEBIAN/preinst
	echo "#!/bin/sh" >$@/DEBIAN/postrm
	echo '[ "$$1" = "remove" ] || exit 0' >>$@/DEBIAN/postrm
	echo 'dpkg-divert --remove --rename $(common_file)' >>$@/DEBIAN/postrm
	chmod 755 $@/DEBIAN/preinst $@/DEBIAN/postrm

test-coinstall-diversions: ma-setup
	# Test shared files with a diversion
	$(DPKG_DIVERT_ADD) --local --divert $(common_file).real $(common_file)
	$(MAKE) test-coinstall-shared-files common_file=$(common_file).real
	$(DPKG_DIVERT_DEL) --local --divert $(common_file).real $(common_file)
	$(DPKG_INSTALL) pkg-ma-diversion_1.0_all.deb
	$(MAKE) test-coinstall-shared-files common_file=$(common_file).distrib
	$(DPKG_PURGE) pkg-ma-diversion
	# Test with another package providing a conflicting file
	# and the multiarch packages registering the diversion
	$(DPKG_INSTALL) pkg-ma-diverted_1.0_all.deb
	$(DPKG_INSTALL) pkg-ma-shared-diversion_1.0_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-shared-diversion_1.0_$(FOREIGN_ARCH).deb
	grep -q "pkg-ma-diverted" "$(DPKG_INSTDIR)$(common_file).distrib"
	grep -q "pkg-ma-shared-diversion" "$(DPKG_INSTDIR)$(common_file)"
	$(DPKG_PURGE) pkg-ma-shared-diversion:$(NATIVE_ARCH)
	$(call stdout_has,$(DPKG_DIVERT) --list,$(common_file).distrib)
	$(DPKG_PURGE) pkg-ma-shared-diversion:$(FOREIGN_ARCH)
	! $(call stdout_has,$(DPKG_DIVERT) --list,$(common_file).distrib)
	$(DPKG_PURGE) pkg-ma-diverted

test-dpkg-output: ma-setup
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-file_$(NATIVE_ARCH).deb
	$(DPKG_INSTALL) pkg-ma-shared-files_1.0_orig-file_$(FOREIGN_ARCH).deb
	$(call stdout_has,$(DPKG_QUERY) -W,^pkg-ma-shared-files:$(NATIVE_ARCH)	1.0$)
	$(call stdout_has,$(DPKG_QUERY) -W,^pkg-ma-shared-files:$(FOREIGN_ARCH)	1.0$)
	! $(call stdout_is,$(DPKG_QUERY) -f'$${binary:Package}' -W pkg-ma-shared-files,pkg-ma-shared-files)
	$(call stdout_is,$(DPKG_QUERY) -f'$${binary:Package}' -W pkg-ma-shared-files:$(NATIVE_ARCH),pkg-ma-shared-files:$(NATIVE_ARCH))
	$(call stdout_is,$(DPKG_QUERY) -f'$${binary:Package}' -W pkg-ma-shared-files:$(FOREIGN_ARCH),pkg-ma-shared-files:$(FOREIGN_ARCH))
	! $(call stdout_is,$(DPKG_QUERY) -f'$${Package}' -W pkg-ma-shared-files,pkg-ma-shared-files)
	$(call stdout_is,$(DPKG_QUERY) -f'$${Package}' -W pkg-ma-shared-files:$(NATIVE_ARCH),pkg-ma-shared-files)
	$(call stdout_is,$(DPKG_QUERY) -f'$${Package}' -W pkg-ma-shared-files:$(FOREIGN_ARCH),pkg-ma-shared-files)
	$(call stdout_has,$(DPKG_QUERY) -W,^pkg-ma-shared-files:$(FOREIGN_ARCH)	1.0$)
	$(call stdout_has,$(DPKG_QUERY) --search $(common_file),(, |^)pkg-ma-shared-files:$(NATIVE_ARCH)(, |: ))
	$(call stdout_has,$(DPKG_QUERY) --search $(common_file),(, |^)pkg-ma-shared-files:$(FOREIGN_ARCH)(, |: ))
	$(call stdout_has,$(DPKG_QUERY) --list,^ii  pkg-ma-shared-files:$(NATIVE_ARCH))
	$(call stdout_has,$(DPKG_QUERY) --list,^ii  pkg-ma-shared-files:$(FOREIGN_ARCH) )
	$(call stdout_has,$(DPKG) --get-selections,^pkg-ma-shared-files:$(NATIVE_ARCH)	+install$)
	$(call stdout_has,$(DPKG) --get-selections,^pkg-ma-shared-files:$(FOREIGN_ARCH)	+install$)
	echo "pkg-ma-shared-files:$(NATIVE_ARCH)	hold" | $(BEROOT) $(DPKG) --set-selections
	$(call stdout_has,$(DPKG) --get-selections,^pkg-ma-shared-files:$(NATIVE_ARCH)	+hold$)
	$(call stdout_has,$(DPKG) --get-selections,^pkg-ma-shared-files:$(FOREIGN_ARCH)	+install$)
	echo "pkg-ma-shared-files:$(FOREIGN_ARCH)	deinstall" | $(BEROOT) $(DPKG) --set-selections
	$(call stdout_has,$(DPKG) --get-selections,^pkg-ma-shared-files:$(NATIVE_ARCH)	+hold$)
	$(call stdout_has,$(DPKG) --get-selections,^pkg-ma-shared-files:$(FOREIGN_ARCH)	+deinstall$)
	echo "pkg-ma-shared-files:$(NATIVE_ARCH)	install" | $(BEROOT) $(DPKG) --set-selections
	$(call stdout_has,$(DPKG) --get-selections,^pkg-ma-shared-files:$(NATIVE_ARCH)	+install$)
	$(call stdout_has,$(DPKG) --get-selections,^pkg-ma-shared-files:$(FOREIGN_ARCH)	+deinstall$)
	$(DPKG_PURGE) pkg-ma-shared-files:$(NATIVE_ARCH) pkg-ma-shared-files:$(FOREIGN_ARCH)

pkg-ma-db-layout_1.0_$(NATIVE_ARCH):: pkg-template
	touch pkg-ma-db-layout_1.0_$(NATIVE_ARCH)/DEBIAN/foo-1

pkg-ma-db-layout_2.0_$(NATIVE_ARCH):: pkg-template
	touch pkg-ma-db-layout_2.0_$(NATIVE_ARCH)/DEBIAN/foo-2

test-db-layout: ma-setup
	$(DPKG_INSTALL) pkg-ma-db-layout_1.0_$(NATIVE_ARCH).deb
	test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout.list"
	test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout.foo-1"
	$(DPKG_INSTALL) pkg-ma-db-layout_2.0_$(NATIVE_ARCH).deb
	! test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout.list"
	! test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout.foo-1"
	test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout:$(NATIVE_ARCH).list"
	test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout:$(NATIVE_ARCH).foo-2"
	$(DPKG_INSTALL) pkg-ma-db-layout_1.0_$(NATIVE_ARCH).deb
	test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout.list"
	test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout.foo-1"
	! test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout:$(NATIVE_ARCH).list"
	! test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout:$(NATIVE_ARCH).foo-2"
	$(DPKG_PURGE) pkg-ma-db-layout
	# Test the upgrade of the db layout
	$(DPKG_INSTALL) pkg-ma-db-layout_2.0_$(NATIVE_ARCH).deb
	rm -f "$(DPKG_ADMINDIR)/info/format"
	mv "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout:$(NATIVE_ARCH).list" "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout.list"
	mv "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout:$(NATIVE_ARCH).foo-2" "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout.foo-2"
	$(call stdout_has,$(DPKG_QUERY) --control-path pkg-ma-db-layout foo-2,/pkg-ma-db-layout\.foo-2$$)
	$(DPKG_CONFIGURE) -a
	grep -q "^1$$" "$(DPKG_ADMINDIR)/info/format"
	test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout:$(NATIVE_ARCH).list"
	test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout:$(NATIVE_ARCH).foo-2"
	! test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout.list"
	! test -e "$(DPKG_ADMINDIR)/info/pkg-ma-db-layout.foo-2"
	$(call stdout_has,$(DPKG_QUERY) --control-path pkg-ma-db-layout foo-2,/pkg-ma-db-layout:$(NATIVE_ARCH)\.foo-2$$)
	$(DPKG_PURGE) pkg-ma-db-layout

test-clean:
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/m-a/triggers"
	$(DPKG_QUERY) -W | grep ^pkg-ma- | (while read pkg ver; do \
	    $(DPKG_PURGE) $$pkg; \
	done)
	-$(DPKG_DIVERT_DEL) --local --divert $(common_file).real $(common_file)
	rm -f "$(DPKG_ADMINDIR)/arch"
