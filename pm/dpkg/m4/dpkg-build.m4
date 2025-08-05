# serial 1
# Copyright © 2010-2014 Guillem Jover <guillem@debian.org>

# DPKG_BUILD_SHARED_LIBS()
# ----------------------
AC_DEFUN([DPKG_BUILD_SHARED_LIBS], [
  AS_IF([test "$enable_shared" = "yes" && test -z "$AUTHOR_TESTING"], [
    AC_MSG_ERROR([building libdpkg as a shared library is not supported])
  ])
])# DPKG_BUILD_SHARED_LIBS

# DPKG_BUILD_RELEASE_DATE()
# -----------------------
AC_DEFUN([DPKG_BUILD_RELEASE_DATE], [
  AC_REQUIRE([DPKG_PROG_PERL])
  TIMESTAMP=$(PERL=$PERL ${CONFIG_SHELL-/bin/sh} "$srcdir/build-aux/run-script" scripts/dpkg-parsechangelog.pl -l"$srcdir/debian/changelog" -STimestamp)
  PACKAGE_RELEASE_DATE=$($PERL -MPOSIX -e "print POSIX::strftime('%Y-%m-%d', gmtime('$TIMESTAMP'));")
  AC_SUBST([PACKAGE_RELEASE_DATE])
])# DPKG_BUILD_RELEASE_DATE

# DPKG_BUILD_PROG(PROG)
# ---------------
# Allow disabling compilation and usage of specific programs.
AC_DEFUN([DPKG_BUILD_PROG], [
  AC_MSG_CHECKING([whether to build $1])
  AC_ARG_ENABLE([$1],
    [AS_HELP_STRING([--disable-$1], [do not build or use $1])],
    [AS_TR_SH([build_$1])=$AS_TR_SH([enable_$1])],
    [AS_TR_SH([build_$1])=yes])
  AM_CONDITIONAL(AS_TR_CPP([BUILD_$1]),
    [test "$AS_TR_SH([build_$1])" = "yes"])
  AS_IF([test "$AS_TR_SH([build_$1])" = "yes"], [
    AC_DEFINE(AS_TR_CPP([BUILD_$1]), [1], [Define to 1 if $1 is compiled.])
  ], [
    AC_DEFINE(AS_TR_CPP([BUILD_$1]), [0])
  ])
  AC_MSG_RESULT([$AS_TR_SH([build_$1])])
])# DPKG_BUILD_PROG

# DPKG_BUILD_DEVEL_DOCS()
# ---------------------
# Select what type of documentation to build. Either for development including
# all symbol references, and extracting everything, or production documentation.
AC_DEFUN([DPKG_BUILD_DEVEL_DOCS], [
  AC_ARG_ENABLE([devel-docs],
    [AS_HELP_STRING([--disable-devel-docs], [build release docs])],
    [build_devel_docs=$enable_devel_docs],
    [build_devel_docs=yes]
  )
  AS_IF([test "$build_devel_docs" = "yes"], [
    AC_SUBST([BUILD_DEVEL_DOCS], [YES])
  ], [
    AC_SUBST([BUILD_DEVEL_DOCS], [NO])
  ])
])# DPKG_BUILD_DOCS_MODE

# DPKG_WITH_DIR(DIR, DEFAULT, DESCRIPTION)
# -------------
# Allow specifying alternate directories.
AC_DEFUN([DPKG_WITH_DIR], [
  $1="$2"
  AC_ARG_WITH([$1], [AS_HELP_STRING([--with-$1=DIR], [$3])], [
    AS_CASE([$with_$1],
      [""], [AC_MSG_ERROR([invalid $1 specified])],
      [$1="$with_$1"])
  ])
  AC_SUBST([$1])
])# DPKG_WITH_DIR

# DPKG_DEB_COMPRESSOR(COMP)
# -------------------
# Change default «dpkg-deb --build» compressor.
AC_DEFUN([DPKG_DEB_COMPRESSOR], [
  AC_ARG_WITH([deb-compressor],
    [AS_HELP_STRING([--with-deb-compressor=COMP],
      [change default dpkg-deb build compressor])],
    [with_deb_compressor=$withval], [with_deb_compressor=$1])
  AS_CASE([$with_deb_compressor],
    [gzip|xz], [:],
    [AC_MSG_ERROR([unsupported default compressor $with_deb_compressor])])
  AC_SUBST([DEB_DEFAULT_COMPRESSOR], [$with_deb_compressor])
  AC_DEFINE_UNQUOTED([DEB_DEFAULT_COMPRESSOR],
    [COMPRESSOR_TYPE_]AS_TR_CPP(${with_deb_compressor}),
    [default dpkg-deb build compressor])
]) # DPKG_DEB_COMPRESSOR

# DPKG_DIST_IS_RELEASE()
# --------------------
# Check whether we are preparing a distribution tarball for a release, and
# set PACKAGE_DIST_IS_RELEASE accordingly.
AC_DEFUN([DPKG_DIST_IS_RELEASE], [
  AS_IF([echo $PACKAGE_VERSION | grep -q -v '[-]'], [
    dpkg_dist_is_release=1
  ], [
    dpkg_dist_is_release=0
  ])
  AM_CONDITIONAL([PACKAGE_DIST_IS_RELEASE],
    [test "$dpkg_dist_is_release" -eq 1])
  AC_SUBST([PACKAGE_DIST_IS_RELEASE], [$dpkg_dist_is_release])
])# DPKG_DIST_IS_RELEASE

# DPKG_DIST_CHECK(COND, ERROR)
# ---------------
# Check if the condition is fulfilled when preparing a distribution tarball.
AC_DEFUN([DPKG_DIST_CHECK], [
  AS_IF([test ! -f "$srcdir/.dist-version" && $1], [
    AC_MSG_ERROR([not building from distributed tarball, $2])
  ])
])# DPKG_DIST_CHECK
