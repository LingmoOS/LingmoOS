AC_INIT([Lingmo OS], [1.0], [team@lingmo.org])

AM_INIT_AUTOMAKE([foreign subdir-objects])

AC_PROG_CC

BUILD_DIR="buildpkg"
BUILD_TP_DIR="buildpkg_tp"
THIRD_PARTY_DIR="third-party"
CPU_CORES=$(nproc)

AC_SUBST(BUILD_DIR)
AC_SUBST(BUILD_TP_DIR)
AC_SUBST(THIRD_PARTY_DIR)
AC_SUBST(CPU_CORES)

AC_CONFIG_FILES([Makefile])
AC_OUTPUT