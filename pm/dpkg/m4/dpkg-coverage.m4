# serial 1
# Copyright © 2010 Guillem Jover <guillem@debian.org>

# DPKG_CODE_COVERAGE
# ------------------
# Add configuration option to enable code coverage support.
AC_DEFUN([DPKG_CODE_COVERAGE], [
  AC_ARG_ENABLE([coverage],
    [AS_HELP_STRING([--enable-coverage], [whether to enable code coverage])],
    [], [enable_coverage=no])
  AM_CONDITIONAL([COVERAGE_ENABLED], [test x$enable_coverage = xyes])

  AS_IF([test "$enable_coverage" = "yes"], [
     AS_IF([test "$GCC" = "no"], [
       AC_MSG_ERROR([not compiling with gcc, which is required for C coverage support])
     ])

     AC_CHECK_PROGS([GCOV], [gcov])
     AS_IF([test -z "$GCOV"], [
       AC_MSG_ERROR([missing gcov, which is required for C coverage support])
     ])

     AC_CHECK_PROGS([LCOV], [lcov])
     AS_IF([test -z "$LCOV"], [
       AC_MSG_ERROR([missing lcov, which is required for C coverage support])
     ])

     AC_CHECK_PROGS([LCOV_GENHTML], [genhtml])
     AS_IF([test -z "$LCOV_GENHTML"], [
       AC_MSG_ERROR([missing genhtml, which is required for C coverage support])
     ])

     CFLAGS="$CFLAGS --coverage -O0"
     LDFLAGS="$LDFLAGS --coverage -O0"

     AC_MSG_CHECKING([for Devel::Cover perl module])
     AS_IF([$($PERL -e "require Devel::Cover;" 2>/dev/null)], [
       PERL_COVERAGE="-MDevel::Cover"
       AC_SUBST(PERL_COVERAGE)
       AC_MSG_RESULT([ok])
     ], [
       AC_MSG_ERROR([Devel::Cover perl module is required for coverage support])
     ])
     AC_CHECK_PROGS([PERL_COVER], [cover])
     AS_IF([test -z "$PERL_COVER"], [
       AC_MSG_ERROR([missing cover, which is required for perl coverage support])
     ])
  ])
  AC_MSG_CHECKING([whether to build with code coverage])
  AC_MSG_RESULT([$enable_coverage])
])
