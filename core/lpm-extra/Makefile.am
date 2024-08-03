SUBDIRS = src lib test completions
if WANT_DOC
SUBDIRS += doc
endif

DIST_SUBDIRS = $(SUBDIRS)

DISTCHECK_CONFIGURE_FLAGS = --prefix=/usr --sysconfdir=/etc --localstatedir=/var --enable-doc --disable-git-version

EXTRA_DIST = README.md AUTHORS.md

TESTS = \
	test/pacsorttest.sh


TEST_SUITE_LOG = test/test-suite.log
AM_TESTS_ENVIRONMENT = \
	PMTEST_BIN_DIR=$(top_builddir)/src/; export PMTEST_BIN_DIR;
LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
	$(top_srcdir)/build-aux/tap-driver.sh
