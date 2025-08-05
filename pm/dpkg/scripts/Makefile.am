## Process this file with automake to produce Makefile.in

SUBDIRS = mk po

EXTRA_DIST =
CLEANFILES =

nobase_dist_perllib_DATA = \
	Dpkg/Arch.pm \
	Dpkg/Archive/Ar.pm \
	Dpkg/BuildAPI.pm \
	Dpkg/BuildDriver.pm \
	Dpkg/BuildDriver/DebianRules.pm \
	Dpkg/BuildEnv.pm \
	Dpkg/BuildFlags.pm \
	Dpkg/BuildInfo.pm \
	Dpkg/BuildOptions.pm \
	Dpkg/BuildProfiles.pm \
	Dpkg/BuildTree.pm \
	Dpkg/BuildTypes.pm \
	Dpkg/Build/Info.pm \
	Dpkg/Changelog.pm \
	Dpkg/Changelog/Debian.pm \
	Dpkg/Changelog/Entry.pm \
	Dpkg/Changelog/Entry/Debian.pm \
	Dpkg/Changelog/Parse.pm \
	Dpkg/Checksums.pm \
	Dpkg/Compression.pm \
	Dpkg/Compression/FileHandle.pm \
	Dpkg/Compression/Process.pm \
	Dpkg/Conf.pm \
	Dpkg/Control.pm \
	Dpkg/Control/Changelog.pm \
	Dpkg/Control/FieldsCore.pm \
	Dpkg/Control/Fields.pm \
	Dpkg/Control/Info.pm \
	Dpkg/Control/HashCore.pm \
	Dpkg/Control/HashCore/Tie.pm \
	Dpkg/Control/Hash.pm \
	Dpkg/Control/Tests.pm \
	Dpkg/Control/Tests/Entry.pm \
	Dpkg/Control/Types.pm \
	Dpkg/Deps.pm \
	Dpkg/Deps/AND.pm \
	Dpkg/Deps/KnownFacts.pm \
	Dpkg/Deps/Multiple.pm \
	Dpkg/Deps/OR.pm \
	Dpkg/Deps/Simple.pm \
	Dpkg/Deps/Union.pm \
	Dpkg/Dist/Files.pm \
	Dpkg/ErrorHandling.pm \
	Dpkg/Exit.pm \
	Dpkg/File.pm \
	Dpkg/Getopt.pm \
	Dpkg/Gettext.pm \
	Dpkg/Index.pm \
	Dpkg/Interface/Storable.pm \
	Dpkg/IPC.pm \
	Dpkg/Lock.pm \
	Dpkg/OpenPGP.pm \
	Dpkg/OpenPGP/Backend.pm \
	Dpkg/OpenPGP/Backend/GnuPG.pm \
	Dpkg/OpenPGP/Backend/SOP.pm \
	Dpkg/OpenPGP/Backend/Sequoia.pm \
	Dpkg/OpenPGP/ErrorCodes.pm \
	Dpkg/OpenPGP/KeyHandle.pm \
	Dpkg/Package.pm \
	Dpkg/Path.pm \
	Dpkg/Shlibs.pm \
	Dpkg/Shlibs/Objdump.pm \
	Dpkg/Shlibs/Objdump/Object.pm \
	Dpkg/Shlibs/Symbol.pm \
	Dpkg/Shlibs/SymbolFile.pm \
	Dpkg/Shlibs/Cppfilt.pm \
	Dpkg/Source/Archive.pm \
	Dpkg/Source/BinaryFiles.pm \
	Dpkg/Source/Format.pm \
	Dpkg/Source/Functions.pm \
	Dpkg/Source/Package.pm \
	Dpkg/Source/Package/V1.pm \
	Dpkg/Source/Package/V2.pm \
	Dpkg/Source/Package/V3/Bzr.pm \
	Dpkg/Source/Package/V3/Custom.pm \
	Dpkg/Source/Package/V3/Native.pm \
	Dpkg/Source/Package/V3/Git.pm \
	Dpkg/Source/Package/V3/Quilt.pm \
	Dpkg/Source/Patch.pm \
	Dpkg/Source/Quilt.pm \
	Dpkg/Substvars.pm \
	Dpkg/Vendor.pm \
	Dpkg/Vendor/Debian.pm \
	Dpkg/Vendor/Default.pm \
	Dpkg/Vendor/Devuan.pm \
	Dpkg/Vendor/PureOS.pm \
	Dpkg/Vendor/Ubuntu.pm \
	Dpkg/Version.pm \
	Dpkg.pm \
	# EOL

EXTRA_DIST += \
	Test/Dpkg.pm \
	# EOL

bin_SCRIPTS = \
	dpkg-architecture \
	dpkg-buildflags \
	dpkg-buildapi \
	dpkg-buildpackage \
	dpkg-buildtree \
	dpkg-checkbuilddeps \
	dpkg-distaddfile \
	dpkg-genbuildinfo \
	dpkg-genchanges \
	dpkg-gencontrol \
	dpkg-gensymbols \
	dpkg-mergechangelogs \
	dpkg-name \
	dpkg-parsechangelog \
	dpkg-scanpackages \
	dpkg-scansources \
	dpkg-shlibdeps \
	dpkg-source \
	dpkg-vendor \
	# EOL

EXTRA_DIST += \
	dpkg-ar.pl \
	dpkg-architecture.pl \
	dpkg-buildapi.pl \
	dpkg-buildflags.pl \
	dpkg-buildpackage.pl \
	dpkg-buildtree.pl \
	dpkg-checkbuilddeps.pl \
	dpkg-distaddfile.pl \
	dpkg-genbuildinfo.pl \
	dpkg-genchanges.pl \
	dpkg-gencontrol.pl \
	dpkg-gensymbols.pl \
	dpkg-mergechangelogs.pl \
	dpkg-name.pl \
	dpkg-parsechangelog.pl \
	dpkg-scanpackages.pl \
	dpkg-scansources.pl \
	dpkg-shlibdeps.pl \
	dpkg-source.pl \
	dpkg-vendor.pl \
	# EOL

CLEANFILES += \
	$(sbin_SCRIPTS) \
	$(bin_SCRIPTS) \
	# EOL

dist_bashcompletions_DATA = \
	# EOL

dist_zshcompletions_DATA = \
	completion/zsh/_dpkg-parsechangelog \
	# EOL

# Keep it even if empty to have man3dir correctly set
man3_MANS =

man_perl_section = 3perl

SUFFIXES =

include $(top_srcdir)/build-aux/subst.am

POD2MAN_OPTS = \
	--utf8 \
	--release=$(PACKAGE_VERSION) \
	--date='$(PACKAGE_RELEASE_DATE)' \
	# EOL

install-data-local:
if BUILD_POD_DOC
# Install manual pages for perl modules manually as make can't deal
# properly with filenames containing colons
# Remove empty manual pages (perl modules not documented)
	$(MKDIR_P) $(DESTDIR)$(man3dir)
	for module in $(nobase_dist_perllib_DATA); do \
	  name=`echo $$module | $(SED) -e 's|/|::|g' -e 's/\.pm$$//'`; \
	  $(POD2MAN) $(POD2MAN_OPTS) --center=libdpkg-perl \
	    --name="$$name" --section=$(man_perl_section) $(srcdir)/$$module \
	    >$(DESTDIR)$(man3dir)/$$name.$(man_perl_section); \
	  if ! [ -s "$(DESTDIR)$(man3dir)/$$name.$(man_perl_section)" ]; then \
	    echo "error: empty $(man3dir)/$$name.$(man_perl_section)" 2>&1; \
	    exit 1; \
	  fi; \
	done
endif

install-data-hook:
	$(subst_perl_file) $(DESTDIR)$(perllibdir)/Dpkg.pm

uninstall-local:
if BUILD_POD_DOC
	for module in $(nobase_dist_perllib_DATA); do \
	  rm -f $(DESTDIR)$(man3dir)/`echo $$module | $(SED) -e 's|/|::|g' -e 's/\.pm$$/.$(man_perl_section)/'`; \
	done
endif

coverage: check
	$(PERL_COVER) -silent \
	  -ignore_re '^t/.*\.t' \
	  -outputdir $(top_builddir)/doc/coverage/scripts
	: # Cleanup coverage report index
	$(PERL) -i -p -e "s:$(abs_top_builddir)::" \
	  $(top_builddir)/doc/coverage/scripts/coverage.html

coverage-clean:
	rm -rf cover_db

TEST_ENV_VARS = \
	DPKG_PROGTAR=$(TAR) \
	DPKG_PROGPATCH=$(PATCH) \
	DPKG_PROGMAKE=$(MAKE) \
	DPKG_DATADIR=$(top_srcdir)/data \
	DPKG_ORIGINS_DIR=$(srcdir)/t/origins \
	# EOL
TEST_COVERAGE = $(PERL_COVERAGE)

test_tmpdir = t.tmp

test_scripts = \
	t/Dpkg_Version.t \
	t/Dpkg_Arch.t \
	t/Dpkg_Package.t \
	t/Dpkg_Shlibs_Cppfilt.t \
	t/Dpkg_Shlibs.t \
	t/Dpkg_BuildAPI.t \
	t/Dpkg_BuildEnv.t \
	t/Dpkg_BuildFlags.t \
	t/Dpkg_BuildFlags_Ubuntu.t \
	t/Dpkg_BuildInfo.t \
	t/Dpkg_BuildOptions.t \
	t/Dpkg_BuildProfiles.t \
	t/Dpkg_BuildTree.t \
	t/Dpkg_BuildTypes.t \
	t/Dpkg_Checksums.t \
	t/Dpkg_ErrorHandling.t \
	t/Dpkg_Exit.t \
	t/Dpkg_File.t \
	t/Dpkg_Lock.t \
	t/Dpkg_Getopt.t \
	t/Dpkg_Gettext.t \
	t/Dpkg_Conf.t \
	t/Dpkg_Deps.t \
	t/Dpkg_Path.t \
	t/Dpkg_Interface_Storable.t \
	t/Dpkg_OpenPGP_KeyHandle.t \
	t/Dpkg_OpenPGP.t \
	t/Dpkg_Vendor.t \
	t/Dpkg_Changelog.t \
	t/Dpkg_Changelog_Ubuntu.t \
	t/Dpkg_Control_Fields.t \
	t/Dpkg_Control.t \
	t/Dpkg_Control_Tests.t \
	t/Dpkg_Index.t \
	t/Dpkg_Substvars.t \
	t/Dpkg_IPC.t \
	t/Dpkg_Compression.t \
	t/Dpkg_Source_Functions.t \
	t/Dpkg_Source_Patch.t \
	t/Dpkg_Source_Quilt.t \
	t/Dpkg_Source_Format.t \
	t/Dpkg_Source_Archive.t \
	t/Dpkg_Source_Package.t \
	t/Dpkg_Dist_Files.t \
	t/dpkg_source.t \
	t/dpkg_buildpackage.t \
	t/dpkg_buildtree.t \
	t/dpkg_mergechangelogs.t \
	t/mk.t \
	# EOL

test_data = \
	t/Dpkg_BuildAPI/ctrl-api-default \
	t/Dpkg_BuildAPI/ctrl-api-desync \
	t/Dpkg_BuildAPI/ctrl-api-explicit \
	t/Dpkg_BuildAPI/ctrl-api-gt-max \
	t/Dpkg_BuildAPI/ctrl-api-no-int \
	t/Dpkg_BuildAPI/ctrl-api-no-ver \
	t/Dpkg_BuildAPI/ctrl-api-rel-noeq \
	t/Dpkg_Changelog/countme \
	t/Dpkg_Changelog/date-format \
	t/Dpkg_Changelog/fields \
	t/Dpkg_Changelog/misplaced-tz \
	t/Dpkg_Changelog/regressions \
	t/Dpkg_Changelog/shadow \
	t/Dpkg_Changelog/stop-modeline \
	t/Dpkg_Changelog/unreleased \
	t/Dpkg_Checksums/data-1 \
	t/Dpkg_Checksums/data-2 \
	t/Dpkg_Checksums/empty \
	t/Dpkg_Conf/config-mixed \
	t/Dpkg_Control/bogus-armor-double.dsc \
	t/Dpkg_Control/bogus-armor-formfeed.dsc \
	t/Dpkg_Control/bogus-armor-inline.dsc \
	t/Dpkg_Control/bogus-armor-nested.dsc \
	t/Dpkg_Control/bogus-armor-no-sig.dsc \
	t/Dpkg_Control/bogus-armor-spaces.dsc \
	t/Dpkg_Control/bogus-armor-trail.dsc \
	t/Dpkg_Control/bogus-unsigned.dsc \
	t/Dpkg_Control/control-1 \
	t/Dpkg_Control_Tests/tests-missing-fields \
	t/Dpkg_Control_Tests/tests-plain-text \
	t/Dpkg_Control_Tests/tests-valid \
	t/Dpkg_Dist_Files/files-byhand \
	t/Dpkg_Dist_Files/files-fragment-a \
	t/Dpkg_Dist_Files/files-fragment-b \
	t/Dpkg_File/append-me \
	t/Dpkg_File/dump-partial \
	t/Dpkg_File/slurp-me \
	t/Dpkg_OpenPGP/dpkg-test-pub.asc \
	t/Dpkg_OpenPGP/dpkg-test-sec.asc \
	t/Dpkg_OpenPGP/sign-file \
	t/Dpkg_OpenPGP/sign-file-inline.asc \
	t/Dpkg_OpenPGP/sign-file-inline.sig \
	t/Dpkg_OpenPGP/sign-file.asc \
	t/Dpkg_OpenPGP/sign-file.sig \
	t/Dpkg_Shlibs/anglebrackets.cpp \
	t/Dpkg_Shlibs/anglebrackets.symbols \
	t/Dpkg_Shlibs/basictags.c \
	t/Dpkg_Shlibs/basictags.symbols \
	t/Dpkg_Shlibs/ld.so.conf \
	t/Dpkg_Shlibs/ld.so.conf.d/inf_recurse.conf \
	t/Dpkg_Shlibs/ld.so.conf.d/normal.conf \
	t/Dpkg_Shlibs/ld.so.conf.d/recursive.conf \
	t/Dpkg_Shlibs/ld.so.conf_2 \
	t/Dpkg_Shlibs/objdump.anglebrackets \
	t/Dpkg_Shlibs/objdump.basictags-amd64 \
	t/Dpkg_Shlibs/objdump.basictags-i386 \
	t/Dpkg_Shlibs/objdump.basictags-mips \
	t/Dpkg_Shlibs/objdump.dbd-pg \
	t/Dpkg_Shlibs/objdump.glib-ia64 \
	t/Dpkg_Shlibs/objdump.internal \
	t/Dpkg_Shlibs/objdump.libc6-2.3 \
	t/Dpkg_Shlibs/objdump.libc6-2.6 \
	t/Dpkg_Shlibs/objdump.ls \
	t/Dpkg_Shlibs/objdump.patterns \
	t/Dpkg_Shlibs/objdump.space \
	t/Dpkg_Shlibs/objdump.spacesyms \
	t/Dpkg_Shlibs/patterns.cpp \
	t/Dpkg_Shlibs/patterns.map \
	t/Dpkg_Shlibs/patterns.symbols \
	t/Dpkg_Shlibs/spacesyms-c-gen.pl \
	t/Dpkg_Shlibs/spacesyms-o-map.pl \
	t/Dpkg_Shlibs/spacesyms.map \
	t/Dpkg_Shlibs/symbols.fake-1 \
	t/Dpkg_Shlibs/symbols.fake-2 \
	t/Dpkg_Shlibs/symbols.fake-3 \
	t/Dpkg_Shlibs/symbols.include-1 \
	t/Dpkg_Shlibs/symbols.include-2 \
	t/Dpkg_Shlibs/symbols.include-3 \
	t/Dpkg_Shlibs/symbols.internal \
	t/Dpkg_Shlibs/symbols.internal-allow \
	t/Dpkg_Shlibs/symbols.internal-allow-groups \
	t/Dpkg_Shlibs/symbols.internal-filter \
	t/Dpkg_Source_Patch/c-style.patch \
	t/Dpkg_Source_Patch/ghost-hunk.patch \
	t/Dpkg_Source_Patch/indent-header.patch \
	t/Dpkg_Source_Patch/index-+++.patch \
	t/Dpkg_Source_Patch/index-alone.patch \
	t/Dpkg_Source_Patch/index-inert.patch \
	t/Dpkg_Source_Patch/partial.patch \
	t/Dpkg_Source_Quilt/parse/debian/patches/series \
	t/Dpkg_Source_Package/package_1.0.orig.tar \
	t/Dpkg_Source_Package/package_1.0.orig.tar.asc \
	t/Dpkg_Source_Package/package_1.0.orig.tar.sig \
	t/Dpkg_Substvars/substvars-req \
	t/Dpkg_Substvars/substvars1 \
	t/Dpkg_Substvars/substvars2 \
	t/dpkg_buildpackage/dpkgdb/status \
	t/dpkg_buildpackage/hook \
	t/dpkg_buildpackage/test-source_0.dsc \
	t/dpkg_buildpackage/test-source_0_all.changes \
	t/dpkg_buildpackage/test-source_0_all.hook \
	t/dpkg_buildpackage/test-source_0_any.changes \
	t/dpkg_buildpackage/test-source_0_any.hook \
	t/dpkg_buildpackage/test-source_0_binary.changes \
	t/dpkg_buildpackage/test-source_0_binary.hook \
	t/dpkg_buildpackage/test-source_0_full.changes \
	t/dpkg_buildpackage/test-source_0_full.hook \
	t/dpkg_buildpackage/test-source_0_source.changes \
	t/dpkg_buildpackage/test-source_0_source.hook \
	t/dpkg_buildtree/src-build-api-v0/debian/control \
	t/dpkg_buildtree/src-build-api-v1/debian/control \
	t/dpkg_buildtree/src-rrr-binary-targets/debian/control \
	t/dpkg_buildtree/src-rrr-dpkg-target-binary/debian/control \
	t/dpkg_buildtree/src-rrr-dpkg-target-custom/debian/control \
	t/dpkg_buildtree/src-rrr-dpkg-target-subcommand/debian/control \
	t/dpkg_buildtree/src-rrr-dpkg-unknown-keyword/debian/control \
	t/dpkg_buildtree/src-rrr-impl-specific-keyword/debian/control \
	t/dpkg_buildtree/src-rrr-missing/debian/control \
	t/dpkg_buildtree/src-rrr-no/debian/control \
	t/dpkg_mergechangelogs/ch-a \
	t/dpkg_mergechangelogs/ch-b \
	t/dpkg_mergechangelogs/ch-badver-a \
	t/dpkg_mergechangelogs/ch-badver-b \
	t/dpkg_mergechangelogs/ch-badver-merged \
	t/dpkg_mergechangelogs/ch-badver-old \
	t/dpkg_mergechangelogs/ch-merged \
	t/dpkg_mergechangelogs/ch-merged-basic \
	t/dpkg_mergechangelogs/ch-merged-pr \
	t/dpkg_mergechangelogs/ch-merged-pr-basic \
	t/dpkg_mergechangelogs/ch-old \
	t/dpkg_mergechangelogs/ch-unreleased-a \
	t/dpkg_mergechangelogs/ch-unreleased-b \
	t/dpkg_mergechangelogs/ch-unreleased-merged \
	t/dpkg_mergechangelogs/ch-unreleased-merged-basic \
	t/dpkg_mergechangelogs/ch-unreleased-old \
	t/dpkg_source/testsuite_0.dsc \
	t/dpkg_source/testsuite_1.dsc \
	t/dpkg_source/testsuite_2.dsc \
	t/dpkg_source/testsuite_3.dsc \
	t/mk/architecture.mk \
	t/mk/buildapi.mk \
	t/mk/buildflags.mk \
	t/mk/buildopts.mk \
	t/mk/buildtools.mk \
	t/mk/debian/changelog \
	t/mk/debian/control \
	t/mk/pkg-info.mk \
	t/mk/vendor-v0.mk \
	t/mk/vendor-v1.mk \
	t/mk/vendor.mk \
	t/mock-bin/dpkg \
	t/mock-bin/gcc \
	t/origins/debian \
	t/origins/devuan \
	t/origins/default \
	t/origins/gnewsense \
	t/origins/ubuntu \
	# EOL

EXTRA_DIST += \
	$(test_scripts) \
	$(test_data) \
	# EOL

test_data_objects = \
	t/Dpkg_Shlibs/libobjdump.anglebrackets.so \
	t/Dpkg_Shlibs/libobjdump.basictags-amd64.so \
	t/Dpkg_Shlibs/libobjdump.basictags-i386.so \
	t/Dpkg_Shlibs/libobjdump.basictags-mips.so \
	t/Dpkg_Shlibs/libobjdump.patterns.so \
	t/Dpkg_Shlibs/libobjdump.spacesyms.so \
	t/Dpkg_Shlibs/spacesyms.c \
	t/Dpkg_Shlibs/spacesyms.o \
	# EOL

CLEANFILES += \
	$(test_data_objects) \
	# EOL

$(srcdir)/t/Dpkg_Shlibs/libobjdump.anglebrackets.so: $(srcdir)/t/Dpkg_Shlibs/anglebrackets.cpp
	$(CXX) $(CXXFLAGS) -shared -fPIC -Wl,-soname -Wl,libanglebrackets.so.1 \
	  $< -o $@

$(srcdir)/t/Dpkg_Shlibs/libobjdump.basictags-amd64.so: $(srcdir)/t/Dpkg_Shlibs/basictags.c
	$(CC) $(CFLAGS) -shared -fPIC -Wl,-soname -Wl,libbasictags.so.1 $< \
	  -DAMD64 -o $@

$(srcdir)/t/Dpkg_Shlibs/libobjdump.basictags-mips.so: $(srcdir)/t/Dpkg_Shlibs/basictags.c
	$(CC) $(CFLAGS) -shared -fPIC -Wl,-soname -Wl,libbasictags.so.1 $< \
	  -DMIPS -o $@

$(srcdir)/t/Dpkg_Shlibs/libobjdump.basictags-i386.so: $(srcdir)/t/Dpkg_Shlibs/basictags.c
	$(CC) $(CFLAGS) -shared -fPIC -Wl,-soname -Wl,libbasictags.so.1 $< \
	  -DI386 -o $@

$(srcdir)/t/Dpkg_Shlibs/libobjdump.patterns.so: $(srcdir)/t/Dpkg_Shlibs/patterns.cpp $(srcdir)/t/Dpkg_Shlibs/patterns.map
	$(CXX) $(CXXFLAGS) -shared -fPIC -Wl,-soname -Wl,libpatterns.so.1 \
	  -Wl,--version-script=$(srcdir)/t/Dpkg_Shlibs/patterns.map $< -o $@

$(srcdir)/t/Dpkg_Shlibs/spacesyms.c: $(srcdir)/t/Dpkg_Shlibs/spacesyms-c-gen.pl
	$(srcdir)/t/Dpkg_Shlibs/spacesyms-c-gen.pl > $@

$(srcdir)/t/Dpkg_Shlibs/spacesyms.o: $(srcdir)/t/Dpkg_Shlibs/spacesyms.c $(srcdir)/t/Dpkg_Shlibs/spacesyms-o-map.pl
	$(CC) $(CFLAGS) -c -fPIC -o $@.tmp $<
	$(srcdir)/t/Dpkg_Shlibs/spacesyms-o-map.pl $@.tmp $@
	rm -f $@.tmp

$(srcdir)/t/Dpkg_Shlibs/libobjdump.spacesyms.so: $(srcdir)/t/Dpkg_Shlibs/spacesyms.o $(srcdir)/t/Dpkg_Shlibs/spacesyms.map
	$(CC) -shared -Wl,-soname -Wl,libspacesyms.so.1 \
	  -Wl,--version-script=$(srcdir)/t/Dpkg_Shlibs/spacesyms.map $< -o $@

.PHONY: refresh-test-data

OBJDUMP = objdump -w -f -p -T -R

refresh-test-data: $(test_data_objects)
	$(OBJDUMP) $(srcdir)/t/Dpkg_Shlibs/libobjdump.anglebrackets.so \
	  >$(srcdir)/t/Dpkg_Shlibs/objdump.anglebrackets
	$(OBJDUMP) $(srcdir)/t/Dpkg_Shlibs/libobjdump.basictags-amd64.so \
	  >$(srcdir)/t/Dpkg_Shlibs/objdump.basictags-amd64
	$(OBJDUMP) $(srcdir)/t/Dpkg_Shlibs/libobjdump.basictags-i386.so \
	  >$(srcdir)/t/Dpkg_Shlibs/objdump.basictags-i386
	$(OBJDUMP) $(srcdir)/t/Dpkg_Shlibs/libobjdump.basictags-mips.so \
	  >$(srcdir)/t/Dpkg_Shlibs/objdump.basictags-mips
	$(OBJDUMP) $(srcdir)/t/Dpkg_Shlibs/libobjdump.patterns.so \
	  >$(srcdir)/t/Dpkg_Shlibs/objdump.patterns
	$(OBJDUMP) $(srcdir)/t/Dpkg_Shlibs/libobjdump.spacesyms.so \
	  >$(srcdir)/t/Dpkg_Shlibs/objdump.spacesyms
	$(OBJDUMP) `command -v ls` >$(srcdir)/t/Dpkg_Shlibs/objdump.ls

include $(top_srcdir)/build-aux/tap.am

check-local: tap-check

clean-local: tap-clean coverage-clean
	rm -fr man
