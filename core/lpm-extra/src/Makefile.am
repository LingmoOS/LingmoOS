# enforce that all scripts have a --help and --version option
AUTOMAKE_OPTIONS = std-options
SUBDIRS=

DIST_SUBDIRS = $(SUBDIRS)

# paths set at make time
conffile  = ${sysconfdir}/lpm.conf
dbpath    = ${localstatedir}/lib/lpm/
gpgdir    = ${sysconfdir}/lpm.d/gnupg/

bin_SCRIPTS = \
	$(OURSCRIPTS)

bin_PROGRAMS = pacsort pactree

vim_ftdetect_dir = $(vim_dir)/ftdetect
vim_ftplugin_dir = $(vim_dir)/ftplugin
vim_syntax_dir = $(vim_dir)/syntax

vim_ftdetect__DATA = \
	vim/ftdetect/build.lpm.vim \
	vim/ftdetect/SRCINFO.vim

vim_ftplugin__DATA = \
	vim/ftplugin/build.lpm.vim

vim_syntax__DATA = \
	vim/syntax/build.lpm.vim \
	vim/syntax/SRCINFO.vim

systemd_dir = ${libdir}/systemd/system

systemd__DATA = \
	paccache.service paccache.timer \
	lpm-filesdb-refresh.service lpm-filesdb-refresh.timer

BASHSCRIPTS = \
	checkupdates \
	paccache \
	pacdiff \
	paclist \
	paclog-pkglist \
	pacscripts \
	rankmirrors \
	updpkgsums

PERLSCRIPTS = \
	pacsearch

OURSCRIPTS = \
	$(BASHSCRIPTS) \
	$(PERLSCRIPTS)

OURFILES = \
	paccache.service \
	lpm-filesdb-refresh.service

EXTRA_DIST = \
	checkupdates.sh.in \
	paccache.service.in \
	paccache.sh.in \
	paccache.timer \
	pacdiff.sh.in \
	paclist.sh.in \
	paclog-pkglist.sh.in \
	lpm-filesdb-refresh.service.in \
	lpm-filesdb-refresh.timer \
	pacscripts.sh.in \
	pacsearch.pl.in \
	pacsort.c \
	pactree.c \
	rankmirrors.sh.in \
	updpkgsums.sh.in \
	vim/syntax/build.lpm.vim \
	vim/syntax/SRCINFO.vim \
	vim/ftdetect/build.lpm.vim \
	vim/ftdetect/SRCINFO.vim \
	vim/ftplugin/build.lpm.vim

# Files that should be removed, but which Automake does not know.
MOSTLYCLEANFILES = $(OURSCRIPTS) $(OURFILES) *.tmp

if USE_GIT_VERSION
GIT_VERSION := $(shell sh -c 'git describe --long --abbrev=4 --dirty | sed s/^v//')
REAL_PACKAGE_VERSION = $(GIT_VERSION)
else
REAL_PACKAGE_VERSION = $(PACKAGE_VERSION)
endif

AM_CPPFLAGS = \
	-DLOCALEDIR=\"@localedir@\" \
	-DCONFFILE=\"$(conffile)\" \
	-DDBPATH=\"$(dbpath)\" \
	-DGPGDIR=\"$(gpgdir)\"

AM_CFLAGS = \
	-fstack-protector-all \
	-pedantic \
	-D_GNU_SOURCE \
	$(WARNING_CFLAGS) \
	$(LIBARCHIVE_CFLAGS) \
	$(LIBALPM_CFLAGS)

edit = sed \
	-e 's|@bindir[@]|$(bindir)|g' \
	-e 's|@sysconfdir[@]|$(sysconfdir)|g' \
	-e 's|@localstatedir[@]|$(localstatedir)|g' \
	-e 's|@PACKAGE_VERSION[@]|$(REAL_PACKAGE_VERSION)|g' \
	-e 's|@libmakepkgdir[@]|$(LIBMAKEPKGDIR)|g' \
	-e 's|@SIZECMD[@]|$(SIZECMD)|g' \
	-e 's|@SCRIPTNAME[@]|$@|g' \
	-e '1s|!/bin/bash|!$(BASH_SHELL)|g' \
	-e 's|@configure_input[@]|Generated from $@.sh.in; do not edit by hand.|g'

$(BASHSCRIPTS): Makefile
	$(AM_V_at)$(RM) $@
	$(AM_V_GEN)test -f $(srcdir)/$@.sh.in && m4 -P -I $(srcdir) $(srcdir)/$@.sh.in | $(edit) >$@
	$(AM_V_at)chmod +x,a-w $@
	@$(BASH_SHELL) -O extglob -n $@

$(PERLSCRIPTS): Makefile
	$(AM_V_at)$(RM) $@ $@.tmp
	$(AM_V_GEN)$(edit) $(srcdir)/$@.pl.in >$@.tmp
	$(AM_V_at)chmod +x,a-w $@.tmp
	$(AM_V_at)mv $@.tmp $@

$(OURFILES): Makefile
	$(AM_V_at)$(RM) $@ $@.tmp
	$(AM_V_GEN)$(edit) $(srcdir)/$@.in >$@.tmp
	$(AM_V_at)chmod a-w $@.tmp
	$(AM_V_at)mv $@.tmp $@

all-am: $(OURSCRIPTS) $(OURFILES)

$(BASHSCRIPTS): %: $(srcdir)/%.sh.in
$(PERLSCRIPTS): %: $(srcdir)/%.pl.in

# dependency links
paccache: $(top_srcdir)/lib/size_to_human.sh

pacsort_SOURCES = pacsort.c
pacsort_LDADD = $(LIBARCHIVE_LIBS) $(LIBALPM_LIBS)

pactree_SOURCES = pactree.c
pactree_LDADD = $(LIBARCHIVE_LIBS) $(LIBALPM_LIBS)
