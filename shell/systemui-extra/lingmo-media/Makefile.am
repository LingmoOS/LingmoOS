NULL =

SUBDIRS = data po man 

DISTCHECK_CONFIGURE_FLAGS =			\
	--enable-more-warnings			\
	--enable-compile-warnings=maximum	\
	--enable-deprecated

DIST_SUBDIRS =					\
	data					\
	po 					\
	man					\
	$(NULL)

EXTRA_DIST =					\
	.version				\
	autogen.sh				\
	configure				\
	ChangeLog				\
	COPYING					\
	COPYING-DOCS				\
	build-aux/git-version-gen 		\
	$(INTLTOOL_BUILT:=.in)			\
	$(NULL)

DISTCLEANFILES =				\
	po/.intltool-merge-cache		\
	$(NULL)

ACLOCAL_AMFLAGS = -I m4 ${ACLOCAL_FLAGS}

# see build-aux/git-version-get for details
BUILT_SOURCES = $(top_srcdir)/.version
$(top_srcdir)/.version:
	echo $(VERSION) > $@-t && mv $@-t $@

# Build ChangeLog from GIT  history
ChangeLog:
	$(AM_V_GEN) if test -d $(top_srcdir)/.git; then \
		GIT_DIR="$(top_srcdir)/.git" git log --stat > $@; \
	fi

dist: ChangeLog

.PHONY: ChangeLog

-include $(top_srcdir)/git.mk
