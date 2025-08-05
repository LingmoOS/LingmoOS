TESTS_DEB := pkg-triggers-native pkg-triggers-foreign pkg-triggers-same

include ../Test.mk

TEST_CASES += test-trigger-db-native-to-same
TEST_CASES += test-trigger-db-foreign-to-same
TEST_CASES += test-trigger-db-same-to-same

test-case: $(TEST_CASES)

test-trigger-db-native-to-same: override DPKG_ADMINDIR = db-native
test-trigger-db-native-to-same:
	cp -a tmpl-$(DPKG_ADMINDIR) $(DPKG_ADMINDIR)
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/a-trigger",pkg-triggers)
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/File",/triggers pkg-triggers)
	$(DPKG_INSTALL) pkg-triggers-native.deb
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/a-trigger",pkg-triggers/noawait)
	grep -q "^/triggers pkg-triggers/noawait$$" "$(DPKG_ADMINDIR)/triggers/File"
	$(DPKG_INSTALL) pkg-triggers-same.deb
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/a-trigger",pkg-triggers:amd64/noawait)
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/File",/triggers pkg-triggers:amd64/noawait)
	$(DPKG_PURGE) pkg-triggers

test-trigger-db-foreign-to-same: override DPKG_ADMINDIR = db-foreign
test-trigger-db-foreign-to-same:
	cp -a tmpl-$(DPKG_ADMINDIR) $(DPKG_ADMINDIR)
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/a-trigger",pkg-triggers:other)
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/File",/triggers pkg-triggers:other)
	: # The db needs to be reproducible, so it gets normalized on upgrade.
	$(DPKG_INSTALL) pkg-triggers-foreign.deb
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/a-trigger",pkg-triggers/noawait)
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/File",/triggers pkg-triggers/noawait)
	$(DPKG_INSTALL) pkg-triggers-same.deb
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/a-trigger",pkg-triggers:amd64/noawait)
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/File",/triggers pkg-triggers:amd64/noawait)
	$(DPKG_PURGE) pkg-triggers

test-trigger-db-same-to-same: override DPKG_ADMINDIR = db-same
test-trigger-db-same-to-same:
	cp -a tmpl-$(DPKG_ADMINDIR) $(DPKG_ADMINDIR)
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/a-trigger",pkg-triggers:amd64)
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/File",/triggers pkg-triggers:amd64)
	$(DPKG_INSTALL) pkg-triggers-same.deb
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/a-trigger",pkg-triggers:amd64/noawait)
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/File",/triggers pkg-triggers:amd64/noawait)
	$(DPKG_PURGE) pkg-triggers

test-clean:
	$(DPKG_PURGE) pkg-triggers
	$(RM) -r db-native db-foreign db-same
