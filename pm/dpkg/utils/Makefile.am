## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	-DADMINDIR=\"$(admindir)\" \
	-DLOCALEDIR=\"$(localedir)\" \
	-DLOGDIR=\"$(logdir)\" \
	-DSYSCONFDIR=\"$(sysconfdir)\" \
	-DRUNSTATEDIR=\"$(runstatedir)\" \
	-idirafter $(top_srcdir)/lib/compat \
	-I$(top_builddir) \
	-I$(top_srcdir)/lib \
	# EOL

sbin_PROGRAMS =
bin_PROGRAMS =
dist_bashcompletions_DATA =
polkitactions_DATA =
EXTRA_DIST =
CLEANFILES =

if BUILD_UPDATE_ALTERNATIVES
bin_PROGRAMS += update-alternatives

update_alternatives_CPPFLAGS = \
	-DALT_TMP_EXT=\".dpkg-tmp\" \
	-DADMINDIR_ENVVAR=\"DPKG_ADMINDIR\" \
	-DINSTDIR_ENVVAR=\"DPKG_ROOT\" \
	$(AM_CPPFLAGS) \
	# EOL

update_alternatives_SOURCES = \
	update-alternatives.c \
	# EOL

update_alternatives_LDADD = \
	../lib/compat/libcompat.la \
	$(LIBINTL) \
	# EOL

dist_bashcompletions_DATA +=

polkitactions_DATA += org.dpkg.pkexec.update-alternatives.policy
endif

subst_polkit_rules = "\
	s{[@]bindir[@]}{$(bindir)}g; \
	"

subst_polkit_filter = $(PERL) -p -e $(subst_polkit_rules)

org.dpkg.pkexec.update-alternatives.policy: update-alternatives.polkit.in Makefile
	@test -d `dirname $@` || $(MKDIR_P) `dirname $@`
	$(AM_V_GEN) GETTEXTDATADIR="$(top_srcdir)/po" \
	  $(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o - \
	  | $(subst_polkit_filter) >$@

EXTRA_DIST += \
	README.alternatives \
	update-alternatives.polkit.in \
	# EOL

CLEANFILES += \
	org.dpkg.pkexec.update-alternatives.policy \
	# EOL

if BUILD_START_STOP_DAEMON
sbin_PROGRAMS += start-stop-daemon

start_stop_daemon_SOURCES = \
	start-stop-daemon.c \
	# EOL

start_stop_daemon_LDADD = \
	../lib/compat/libcompat.la \
	$(SOCKET_LIBS) \
	$(PS_LIBS) \
	$(KVM_LIBS) \
	# EOL

dist_bashcompletions_DATA +=
endif

install-data-local:
if BUILD_UPDATE_ALTERNATIVES
	$(MKDIR_P) $(DESTDIR)$(sysconfdir)/alternatives
	$(MKDIR_P) $(DESTDIR)$(admindir)/alternatives
	$(INSTALL_DATA) $(srcdir)/README.alternatives $(DESTDIR)$(sysconfdir)/alternatives/README
endif

uninstall-local:
	rm -f $(DESTDIR)$(sysconfdir)/alternatives/README

TEST_ENV_VARS = \
	DPKG_DATADIR=$(top_srcdir)/data \
	UA_ROOTDIR_ENVVAR=DPKG_ROOT \
	UA_ADMINDIR_ENVVAR=DPKG_ADMINDIR \
	UA_ADMINDIR_DEFAULT=$(admindir) \
	# EOL

test_tmpdir = t.tmp
test_scripts = \
	t/update_alternatives.t \
	# EOL

EXTRA_DIST += \
	$(test_scripts) \
	# EOL

include $(top_srcdir)/build-aux/tap.am

check-local: tap-check

clean-local: tap-clean
