TESTS_DEB := pkg-somefiles

include ../Test.mk

DEB_FILES_COUNT = $(shell dpkg-deb -c pkg-somefiles.deb | wc -l )

TEST_CASES += test-no-filter
TEST_CASES += test-no-doc-sub
TEST_CASES += test-no-doc-all
TEST_CASES += test-no-doc-except-copyright
TEST_CASES += test-no-doc-except-copyright-subdir
TEST_CASES += test-no-doc-except-copyright-and-readme
TEST_CASES += test-include-only
TEST_CASES += test-reinclude-subdir
TEST_CASES += test-same-include-exclude
TEST_CASES += test-upgrade test-help

build-hook:
	ln -fsT pkg-somefiles pkg-somefiles/test/share/doc/pkg-symlinked

clean-hook:
	$(RM) pkg-somefiles/test/share/doc/pkg-symlinked

test-case: $(TEST_CASES)

test-clean:
	$(DPKG_PURGE) pkg-somefiles

test-no-filter:
	# no filter, should have all files
	$(DPKG_INSTALL) pkg-somefiles.deb
	test "`$(DPKG_QUERY) -L pkg-somefiles | wc -l`" = $(DEB_FILES_COUNT)
	$(DPKG_PURGE) pkg-somefiles

test-no-doc-sub:
	# filter out /test/share/doc/*/*; this keeps the actual
	# /test/share/doc/pkg-somefiles dir and
	# /test/share/doc/pkg-symlinked symlink around.
	$(DPKG_INSTALL) \
	  --path-exclude '/test/share/doc/*/*' \
	  pkg-somefiles.deb
	test -L "$(DPKG_INSTDIR)/test/share/doc/pkg-symlinked"
	test -d "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles"
	test "`ls '$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles'`" = ""
	test -f "$(DPKG_INSTDIR)/test/lib/pkg-somefiles/run"
	$(DPKG_PURGE) pkg-somefiles

test-no-doc-all:
	# filter out /test/share/doc/*
	$(DPKG_INSTALL) \
	  --path-exclude '/test/share/doc/*' \
	  pkg-somefiles.deb
	! test -L "$(DPKG_INSTDIR)/test/share/doc/pkg-symlinked"
	! test -d "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles"
	test -f "$(DPKG_INSTDIR)/test/lib/pkg-somefiles/run"
	$(DPKG_PURGE) pkg-somefiles

test-no-doc-except-copyright:
	# filter out /test/share/doc/*/* except copyright
	$(DPKG_INSTALL) \
	  --path-exclude '/test/share/doc/*/*' \
	  --path-include '/test/share/doc/*/copyright' \
	  pkg-somefiles.deb
	test -L "$(DPKG_INSTDIR)/test/share/doc/pkg-symlinked"
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/copyright"
	! test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/html/index.html"
	! test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/README"
	test -f "$(DPKG_INSTDIR)/test/lib/pkg-somefiles/run"
	$(DPKG_PURGE) pkg-somefiles

test-no-doc-except-copyright-subdir:
	# prune the entire doc dir; this triggers the special case that
	# /test/share/doc/pkg-somefiles is matched by the exclude, but still
	# needs to be created due to the following include
	$(DPKG_INSTALL) \
	  --path-exclude '/test/share/doc/*' \
	  --path-include '/test/share/doc/*/copyright' \
	  pkg-somefiles.deb
	test -L "$(DPKG_INSTDIR)/test/share/doc/pkg-symlinked"
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/copyright"
	! test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/html/index.html"
	! test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/README"
	test -f "$(DPKG_INSTDIR)/test/lib/pkg-somefiles/run"
	$(DPKG_PURGE) pkg-somefiles

test-no-doc-except-copyright-and-readme:
	# two includes which revert an exclude, second of which matches
	# several subdirs with one *
	$(DPKG_INSTALL) \
	  --path-exclude '/test/share/doc/*' \
	  --path-include '/test/share/doc/*/copyright' \
	  --path-include '/test*/READ*' \
	  pkg-somefiles.deb
	test -L "$(DPKG_INSTDIR)/test/share/doc/pkg-symlinked"
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/copyright"
	! test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/html/index.html"
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/README"
	test -f "$(DPKG_INSTDIR)/test/lib/pkg-somefiles/run"
	$(DPKG_PURGE) pkg-somefiles

test-include-only:
	# only includes, should be a no-op and have all files
	$(DPKG_INSTALL) \
	  --path-include '/test/*' \
	  --path-include '/test/share/doc' \
	  --path-include '/test/lib/*/*' \
	  pkg-somefiles.deb
	test "`$(DPKG_QUERY) -L pkg-somefiles | wc -l`" = $(DEB_FILES_COUNT)
	test -L "$(DPKG_INSTDIR)/test/share/doc/pkg-symlinked"
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/copyright"
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/html/index.html"
	test -f "$(DPKG_INSTDIR)/test/lib/pkg-somefiles/run"
	$(DPKG_PURGE) pkg-somefiles

test-reinclude-subdir:
	# Exclude a directory tree and reinclude some of its subdirectories
	$(DPKG_INSTALL) \
	  --path-exclude '/test/share/locale/*' \
	  --path-include '/test/share/locale/de/*' \
	  pkg-somefiles.deb
	test "`$(DPKG_QUERY) -L pkg-somefiles | wc -l`" = $(DEB_FILES_COUNT)
	test -d "$(DPKG_INSTDIR)/test/share/locale/de"
	test -f "$(DPKG_INSTDIR)/test/share/locale/de/foo.mo"
	! test -f "$(DPKG_INSTDIR)/test/share/locale/ca/foo.mo"
	! test -f "$(DPKG_INSTDIR)/test/share/locale/es/foo.mo"
	$(DPKG_PURGE) pkg-somefiles

test-same-include-exclude:
	# include the same things than exclude, should be a no-op and have
	# all files
	$(DPKG_INSTALL) \
	  --path-exclude '/test/share/*' \
	  --path-include '/test/share/*' \
	  pkg-somefiles.deb
	test "`$(DPKG_QUERY) -L pkg-somefiles | wc -l`" = $(DEB_FILES_COUNT)
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/html/index.html"
	test -f "$(DPKG_INSTDIR)/test/lib/pkg-somefiles/run"
	$(DPKG_PURGE) pkg-somefiles
	
	# now doubly so
	$(DPKG_INSTALL) \
	  --path-exclude '/test/share/*' \
	  --path-include '/test/share/*' \
	  --path-exclude '/test/share/*' \
	  --path-include '/test/share/*' \
	  pkg-somefiles.deb
	test "`$(DPKG_QUERY) -L pkg-somefiles | wc -l`" = $(DEB_FILES_COUNT)
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/html/index.html"
	test -f "$(DPKG_INSTDIR)/test/lib/pkg-somefiles/run"
	$(DPKG_PURGE) pkg-somefiles

test-upgrade:
	# files are removed/re-added on upgrades
	$(DPKG_INSTALL) pkg-somefiles.deb
	test "`$(DPKG_QUERY) -L pkg-somefiles | wc -l`" = $(DEB_FILES_COUNT)
	test -L "$(DPKG_INSTDIR)/test/share/doc/pkg-symlinked"
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/copyright"
	
	$(DPKG_INSTALL) \
	  --path-exclude '/test/share/doc/*' \
	  pkg-somefiles.deb
	! test -L "$(DPKG_INSTDIR)/test/share/doc/pkg-symlinked"
	! test -d "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles"
	
	$(DPKG_INSTALL) \
	  --path-exclude '/test/share/doc/*' \
	  --path-include '/test/share/doc/*/copyright' \
	  pkg-somefiles.deb
	test -L "$(DPKG_INSTDIR)/test/share/doc/pkg-symlinked"
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/copyright"
	! test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/README"
	
	$(DPKG_INSTALL) pkg-somefiles.deb
	test "`$(DPKG_QUERY) -L pkg-somefiles | wc -l`" = $(DEB_FILES_COUNT)
	test -L "$(DPKG_INSTDIR)/test/share/doc/pkg-symlinked"
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/copyright"
	test -f "$(DPKG_INSTDIR)/test/share/doc/pkg-somefiles/README"
	$(DPKG_PURGE) pkg-somefiles

# --help output explains the options
test-help:
	$(DPKG) --help | grep -q -- --path-include
	$(DPKG) --help | grep -q -- --path-exclude
