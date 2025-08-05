TESTS_DEB := pkg-triggers pkg-triggers-await pkg-triggers-noawait \
             pkg-trig-file pkg-trig-explicit pkg-trig-cmd

include ../Test.mk

TEST_CASES += test-trigger-file
TEST_CASES += test-trigger-explicit
TEST_CASES += test-trigger-cmd
TEST_CASES += test-trigger-await-file
TEST_CASES += test-trigger-await-cmd
TEST_CASES += test-trigger-noawait-file
TEST_CASES += test-trigger-noawait-explicit
TEST_CASES += test-trigger-noawait-cmd
TEST_CASES += test-internal-db

test-case: $(TEST_CASES)

test-trigger-file test-trigger-await-file:
	if [ "$@" = "test-trigger-file" ]; then \
	  $(DPKG_UNPACK) pkg-triggers.deb; \
	else \
	  $(DPKG_UNPACK) pkg-triggers-await.deb; \
	fi
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok" "$(DPKG_INSTDIR)/triggers/ok"
	# When unpacked, triggers are not active
	$(DPKG_INSTALL) --no-triggers pkg-trig-file.deb
	$(call pkg_is_installed,pkg-trig-file)
	$(call pkg_status_is,pkg-triggers,install ok unpacked)
	$(call pkg_field_is,pkg-triggers,Triggers-Pending,)
	! test -f "$(DPKG_INSTDIR)/triggers/ok"
	$(DPKG_CONFIGURE) --no-triggers pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	! test -f "$(DPKG_INSTDIR)/triggers/ok"
	# When installed, triggers can be activated
	$(DPKG_INSTALL) --no-triggers pkg-trig-file.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_field_is,pkg-triggers,Triggers-Pending, /triggers)
	$(call pkg_status_is,pkg-trig-file,install ok triggers-awaited)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	$(call pkg_is_installed,pkg-trig-file)
	test -f "$(DPKG_INSTDIR)/triggers/ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/triggers/ok"
	# Try other ways to run the triggers
	$(DPKG_INSTALL) --no-triggers pkg-trig-file.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-file,install ok triggers-awaited)
	$(BEROOT) $(DPKG) --triggers-only --pending
	$(call pkg_is_installed,pkg-trig-file)
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/triggers/ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/triggers/ok"
	# Again
	$(DPKG_INSTALL) --no-triggers pkg-trig-file.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-file,install ok triggers-awaited)
	$(DPKG_CONFIGURE) --pending
	$(call pkg_is_installed,pkg-trig-file)
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/triggers/ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/triggers/ok"
	# Again, but we're unpacking instead of installing
	$(DPKG_UNPACK) --no-triggers pkg-trig-file.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-file,install ok unpacked)
	$(DPKG_CONFIGURE) pkg-trig-file
	# TODO: Remove ifdef during dpkg 1.18.x cycle, after the apt
	# workaround has been reverted.
ifdef DPKG_HAS_CONFIGURE_WITH_IMPLICIT_TRIGGER_PENDING
	$(call pkg_status_is,pkg-trig-file,install ok triggers-awaited)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
endif
	$(call pkg_is_installed,pkg-trig-file)
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/triggers/ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/triggers/ok"
	# Cleanup
	$(DPKG_PURGE) pkg-triggers
	$(DPKG_PURGE) pkg-trig-file

test-trigger-noawait-file:
	$(DPKG_UNPACK) pkg-triggers-noawait.deb
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok" "$(DPKG_INSTDIR)/triggers/ok"
	# When unpacked, triggers are not active
	$(DPKG_INSTALL) --no-triggers pkg-trig-file.deb
	$(call pkg_is_installed,pkg-trig-file)
	$(call pkg_status_is,pkg-triggers,install ok unpacked)
	$(call pkg_field_is,pkg-triggers,Triggers-Pending,)
	! test -f "$(DPKG_INSTDIR)/triggers/ok"
	$(DPKG_CONFIGURE) --no-triggers pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	! test -f "$(DPKG_INSTDIR)/triggers/ok"
	# When installed, triggers can be activated
	$(DPKG_INSTALL) --no-triggers pkg-trig-file.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_field_is,pkg-triggers,Triggers-Pending, /triggers)
	$(call pkg_is_installed,pkg-trig-file)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/triggers/ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/triggers/ok"
	# Try other ways to run the triggers
	$(DPKG_INSTALL) --no-triggers pkg-trig-file.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_is_installed,pkg-trig-file)
	$(BEROOT) $(DPKG) --triggers-only --pending
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/triggers/ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/triggers/ok"
	# Again
	$(DPKG_INSTALL) --no-triggers pkg-trig-file.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_is_installed,pkg-trig-file)
	$(DPKG_CONFIGURE) --pending
	$(call pkg_is_installed,pkg-trig-file)
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/triggers/ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/triggers/ok"
	# Again, but we're unpacking instead of installing
	$(DPKG_UNPACK) --no-triggers pkg-trig-file.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-file,install ok unpacked)
	$(DPKG_CONFIGURE) --no-triggers pkg-trig-file
	$(call pkg_is_installed,pkg-trig-file)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/triggers/ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/triggers/ok"
	# Cleanup
	$(DPKG_PURGE) pkg-triggers
	$(DPKG_PURGE) pkg-trig-file

test-trigger-explicit:
	$(DPKG_UNPACK) pkg-triggers.deb
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok" "$(DPKG_INSTDIR)/triggers/ok"
	# When unpacked, triggers are not active
	$(DPKG_INSTALL) --no-triggers pkg-trig-explicit.deb
	$(call pkg_is_installed,pkg-trig-explicit)
	$(call pkg_status_is,pkg-triggers,install ok unpacked)
	$(call pkg_field_is,pkg-triggers,Triggers-Pending,)
	! test -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(DPKG_CONFIGURE) --no-triggers pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	! test -f "$(DPKG_INSTDIR)/a-trigger-ok"
	# When installed, triggers can be activated
	$(DPKG_INSTALL) --no-triggers pkg-trig-explicit.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_field_is,pkg-triggers,Triggers-Pending, a-trigger)
	$(call pkg_status_is,pkg-trig-explicit,install ok triggers-awaited)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	$(call pkg_is_installed,pkg-trig-explicit)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	# Try other ways to run the triggers
	$(DPKG_INSTALL) --no-triggers pkg-trig-explicit.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-explicit,install ok triggers-awaited)
	$(BEROOT) $(DPKG) --triggers-only --pending
	$(call pkg_is_installed,pkg-trig-explicit)
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	# Again
	$(DPKG_INSTALL) --no-triggers pkg-trig-explicit.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-explicit,install ok triggers-awaited)
	$(DPKG_CONFIGURE) --pending
	$(call pkg_is_installed,pkg-trig-explicit)
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	# Again, but we're unpacking instead of installing
	$(DPKG_UNPACK) --no-triggers pkg-trig-explicit.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-explicit,install ok unpacked)
	$(DPKG_CONFIGURE) pkg-trig-explicit
	$(call pkg_is_installed,pkg-trig-explicit)
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	# Verify the trigger is activated on each status change
	$(DPKG_UNPACK) --no-triggers pkg-trig-explicit.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-explicit,install ok unpacked)
	$(call pkg_field_is,pkg-trig-explicit,Triggers-Awaited, pkg-triggers)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	$(call pkg_field_is,pkg-trig-explicit,Triggers-Awaited,)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(DPKG_CONFIGURE) --no-triggers pkg-trig-explicit
	$(call pkg_status_is,pkg-trig-explicit,install ok triggers-awaited)
	$(call pkg_field_is,pkg-trig-explicit,Triggers-Awaited, pkg-triggers)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-trig-explicit)
	$(call pkg_is_installed,pkg-triggers)
	# Cleanup
	$(DPKG_PURGE) pkg-triggers
	$(DPKG_PURGE) pkg-trig-explicit

test-trigger-noawait-explicit:
	$(DPKG_INSTALL) pkg-triggers-noawait.deb
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok" "$(DPKG_INSTDIR)/triggers/ok"
	# When installed, triggers can be activated
	$(DPKG_INSTALL) --no-triggers pkg-trig-explicit.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_field_is,pkg-triggers,Triggers-Pending, a-trigger)
	$(call pkg_is_installed,pkg-trig-explicit)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	# Again, but we're unpacking instead of installing
	$(DPKG_UNPACK) --no-triggers pkg-trig-explicit.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-explicit,install ok unpacked)
	$(call pkg_field_is,pkg-trig-explicit,Triggers-Awaited,)
	$(DPKG_CONFIGURE) --no-triggers pkg-trig-explicit
	$(call pkg_is_installed,pkg-trig-explicit)
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(DPKG_CONFIGURE) --pending
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	# Verify the trigger is activated on each status change
	$(DPKG_UNPACK) --no-triggers pkg-trig-explicit.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-explicit,install ok unpacked)
	$(call pkg_field_is,pkg-trig-explicit,Triggers-Awaited,)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(DPKG_CONFIGURE) --no-triggers pkg-trig-explicit
	$(call pkg_is_installed,pkg-trig-explicit)
	$(call pkg_field_is,pkg-trig-explicit,Triggers-Awaited,)
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	# Cleanup
	$(DPKG_PURGE) pkg-triggers
	$(DPKG_PURGE) pkg-trig-explicit

test-trigger-cmd test-trigger-await-cmd:
	if [ "$@" = "test-trigger-await-cmd" ]; then \
	  $(BEROOT) touch "$(DPKG_INSTDIR)/trigger-await"; \
	fi
	$(DPKG_UNPACK) pkg-triggers.deb
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok" "$(DPKG_INSTDIR)/triggers/ok"
	# When unpacked, triggers are not active
	$(BEROOT) touch "$(DPKG_INSTDIR)/trigger-postinst" "$(DPKG_INSTDIR)/trigger-preinst" "$(DPKG_INSTDIR)/trigger-postrm" "$(DPKG_INSTDIR)/trigger-prerm"
	$(DPKG_INSTALL) --no-triggers pkg-trig-cmd.deb
	$(call pkg_is_installed,pkg-trig-cmd)
	$(call pkg_status_is,pkg-triggers,install ok unpacked)
	$(call pkg_field_is,pkg-triggers,Triggers-Pending,)
	! test -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(DPKG_CONFIGURE) --no-triggers pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	! test -f "$(DPKG_INSTDIR)/a-trigger-ok"
	# When installed, triggers can be activated
	$(DPKG_INSTALL) --no-triggers pkg-trig-cmd.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_field_is,pkg-triggers,Triggers-Pending, a-trigger)
	$(call pkg_status_is,pkg-trig-cmd,install ok triggers-awaited)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	$(call pkg_is_installed,pkg-trig-cmd)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-postinst" "$(DPKG_INSTDIR)/trigger-preinst" "$(DPKG_INSTDIR)/trigger-postrm" "$(DPKG_INSTDIR)/trigger-prerm"
	# Call dpkg-trigger in the preinst
	$(BEROOT) touch "$(DPKG_INSTDIR)/trigger-preinst"
	$(DPKG_UNPACK) --no-triggers pkg-trig-cmd.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-cmd,install ok unpacked)
	$(call pkg_field_is,pkg-trig-cmd,Triggers-Awaited, pkg-triggers)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_field_is,pkg-trig-cmd,Triggers-Awaited,)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-preinst"
	# Call dpkg-trigger in the postinst
	$(BEROOT) touch "$(DPKG_INSTDIR)/trigger-postinst"
	$(DPKG_CONFIGURE) --no-triggers pkg-trig-cmd
	$(call pkg_status_is,pkg-trig-cmd,install ok triggers-awaited)
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	$(call pkg_is_installed,pkg-trig-cmd)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-postinst"
	# Call dpkg-trigger in the prerm
	$(BEROOT) touch "$(DPKG_INSTDIR)/trigger-prerm"
	$(DPKG_UNPACK) --no-triggers pkg-trig-cmd.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-cmd,install ok unpacked)
	$(call pkg_field_is,pkg-trig-cmd,Triggers-Awaited, pkg-triggers)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_field_is,pkg-trig-cmd,Triggers-Awaited,)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-prerm"
	# Call dpkg-trigger in the postrm
	$(BEROOT) touch "$(DPKG_INSTDIR)/trigger-postrm"
	$(DPKG_UNPACK) --no-triggers pkg-trig-cmd.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-cmd,install ok unpacked)
	$(call pkg_field_is,pkg-trig-cmd,Triggers-Awaited, pkg-triggers)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_field_is,pkg-trig-cmd,Triggers-Awaited,)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-postrm"
	# Cleanup
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-await"
	$(DPKG_PURGE) pkg-triggers
	$(DPKG_PURGE) pkg-trig-cmd

test-trigger-noawait-cmd:
	$(BEROOT) touch "$(DPKG_INSTDIR)/trigger-noawait"
	$(DPKG_UNPACK) pkg-triggers.deb
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok" "$(DPKG_INSTDIR)/triggers/ok"
	# When unpacked, triggers are not active
	$(BEROOT) touch "$(DPKG_INSTDIR)/trigger-postinst" "$(DPKG_INSTDIR)/trigger-preinst" "$(DPKG_INSTDIR)/trigger-postrm" "$(DPKG_INSTDIR)/trigger-prerm"
	$(DPKG_INSTALL) --no-triggers pkg-trig-cmd.deb
	$(call pkg_is_installed,pkg-trig-cmd)
	$(call pkg_status_is,pkg-triggers,install ok unpacked)
	$(call pkg_field_is,pkg-triggers,Triggers-Pending,)
	! test -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(DPKG_CONFIGURE) --no-triggers pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	! test -f "$(DPKG_INSTDIR)/a-trigger-ok"
	# When installed, triggers can be activated
	$(DPKG_INSTALL) --no-triggers pkg-trig-cmd.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_field_is,pkg-triggers,Triggers-Pending, a-trigger)
	$(call pkg_is_installed,pkg-trig-cmd)
	$(call pkg_field_is,pkg-trig-cmd,Triggers-Awaited,)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-postinst" "$(DPKG_INSTDIR)/trigger-preinst" "$(DPKG_INSTDIR)/trigger-postrm" "$(DPKG_INSTDIR)/trigger-prerm"
	# Call dpkg-trigger in the preinst
	$(BEROOT) touch "$(DPKG_INSTDIR)/trigger-preinst"
	$(DPKG_UNPACK) --no-triggers pkg-trig-cmd.deb
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(call pkg_status_is,pkg-trig-cmd,install ok unpacked)
	$(call pkg_field_is,pkg-trig-cmd,Triggers-Awaited,)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-preinst"
	# Call dpkg-trigger in the postinst
	$(BEROOT) touch "$(DPKG_INSTDIR)/trigger-postinst"
	$(DPKG_CONFIGURE) --no-triggers pkg-trig-cmd
	$(call pkg_is_installed,pkg-trig-cmd)
	$(call pkg_status_is,pkg-triggers,install ok triggers-pending)
	$(BEROOT) $(DPKG) --triggers-only pkg-triggers
	$(call pkg_is_installed,pkg-triggers)
	test -f "$(DPKG_INSTDIR)/a-trigger-ok" && $(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok"
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-postinst"
	# Cleanup
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-noawait"
	$(DPKG_PURGE) pkg-triggers
	$(DPKG_PURGE) pkg-trig-cmd

test-internal-db:
	# test that installing triggers creates the corresponding files
	$(DPKG_INSTALL) pkg-triggers.deb
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/a-trigger",pkg-triggers)
	grep -q "^/triggers pkg-triggers$$" "$(DPKG_ADMINDIR)/triggers/File"
	$(DPKG_INSTALL) pkg-triggers-noawait.deb
	$(call stdout_is,cat "$(DPKG_ADMINDIR)/triggers/a-trigger",pkg-triggers/noawait)
	grep -q "^/triggers pkg-triggers/noawait$$" "$(DPKG_ADMINDIR)/triggers/File"
	# Test that removal cleans up the triggers files
	$(DPKG_PURGE) pkg-triggers
	! test -e "$(DPKG_ADMINDIR)/triggers/a-trigger"
	! test -e "$(DPKG_ADMINDIR)/triggers/File"

test-clean:
	$(DPKG_PURGE) pkg-triggers pkg-trig-file pkg-trig-explicit pkg-trig-cmd
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/a-trigger-ok" "$(DPKG_INSTDIR)/triggers/ok"
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-postinst" "$(DPKG_INSTDIR)/trigger-preinst" "$(DPKG_INSTDIR)/trigger-postrm" "$(DPKG_INSTDIR)/trigger-prerm"
	$(BEROOT) rm -f "$(DPKG_INSTDIR)/trigger-noawait"

