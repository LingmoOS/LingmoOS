# Copyright © 2012 Guillem Jover <guillem@debian.org>

TESTS_DEB :=

include ../Test.mk

PKG_TUPLE = LC_ALL=C $(DPKG_QUERY) -f '$${Package}:$${Architecture}:$${Version}' -W

TEST_CASES += test-field-blank-lines
TEST_CASES += test-field-blank-lines-update
TEST_CASES += test-dupe-multi
TEST_CASES += test-cross-single-update
TEST_CASES += test-cross-multi-update
TEST_CASES += test-bogus-single
TEST_CASES += test-bogus-dupe-same-single
TEST_CASES += test-bogus-dupe-diff-single
TEST_CASES += test-bogus-multi
TEST_CASES += test-bogus-multi-update
TEST_CASES += test-bogus-versioned-provides
TEST_CASES += test-bogus-versioned-provides-update

test-case: $(TEST_CASES)

test-field-blank-lines: override DPKG_ADMINDIR = db-field-blank-lines
test-field-blank-lines:
	$(call stderr_has,$(PKG_TUPLE),warning)

test-field-blank-lines-update: override DPKG_ADMINDIR = db-field-blank-lines-update
test-field-blank-lines-update:
	$(call stderr_has,$(PKG_TUPLE),warning)

test-dupe-multi: override DPKG_ADMINDIR = db-dupe-multi
test-dupe-multi:
	$(call stdout_is,$(PKG_TUPLE),pkg-multi:amd64:1.0)

test-cross-single-update: override DPKG_ADMINDIR = db-cross-single-update
test-cross-single-update:
	$(call stdout_is,$(PKG_TUPLE),pkg-single:armel:3.0)

test-cross-multi-update: override DPKG_ADMINDIR = db-cross-multi-update
test-cross-multi-update:
	$(call stdout_is,$(PKG_TUPLE),pkg-multi:armel:4.0)

test-bogus-dupe-same-single: override DPKG_ADMINDIR = db-bogus-dupe-same-single
test-bogus-dupe-same-single:
	$(call stderr_has,$(PKG_TUPLE),error)

test-bogus-dupe-diff-single: override DPKG_ADMINDIR = db-bogus-dupe-diff-single
test-bogus-dupe-diff-single:
	$(call stderr_has,$(PKG_TUPLE),error)

test-bogus-single: override DPKG_ADMINDIR = db-bogus-single
test-bogus-single:
	$(call stderr_has,$(PKG_TUPLE),error)

test-bogus-multi: override DPKG_ADMINDIR = db-bogus-multi
test-bogus-multi:
	$(call stderr_has,$(PKG_TUPLE),error)

test-bogus-multi-update: override DPKG_ADMINDIR = db-bogus-multi-update
test-bogus-multi-update:
	$(call stderr_has,$(PKG_TUPLE),error)

test-bogus-versioned-provides: override DPKG_ADMINDIR = db-bogus-versioned-provides
test-bogus-versioned-provides:
	$(call stderr_has,$(PKG_TUPLE),warning)

test-bogus-versioned-provides-update: override DPKG_ADMINDIR = db-bogus-versioned-provides-update
test-bogus-versioned-provides-update:
	$(call stderr_has,$(PKG_TUPLE),warning)
