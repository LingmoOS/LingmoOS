# serial 2
# Copyright © 2004 Scott James Remnant <scott@netsplit.com>
# Copyright © 2006-2024 Guillem Jover <guillem@debian.org>

# DPKG_CHECK_COMPILER_FLAG
# ------------------------
AC_DEFUN([DPKG_CHECK_COMPILER_FLAG], [
  m4_define([dpkg_check_flag], m4_bpatsubst([$1], [^-Wno-], [-W]))

  AC_LANG_CASE(
  [C], [
    m4_define([dpkg_compiler], [$CC])
    m4_define([dpkg_varname], [CFLAGS])
    m4_define([dpkg_varname_save], [dpkg_save_CFLAGS])
    m4_define([dpkg_varname_export], [DPKG_COMPILER_CFLAGS])
    m4_pattern_allow([DPKG_COMPILER_CFLAGS])
    AS_VAR_PUSHDEF([dpkg_varname_cache], [dpkg_cv_cflags_$1])
  ],
  [C++], [
    m4_define([dpkg_compiler], [$CXX])
    m4_define([dpkg_varname], [CXXFLAGS])
    m4_define([dpkg_varname_save], [dpkg_save_CXXFLAGS])
    m4_define([dpkg_varname_export], [DPKG_COMPILER_CXXFLAGS])
    m4_pattern_allow([DPKG_COMPILER_CXXFLAGS])
    AS_VAR_PUSHDEF([dpkg_varname_cache], [dpkg_cv_cxxflags_$1])
  ])
  AC_CACHE_CHECK([whether ]dpkg_compiler[ accepts $1], [dpkg_varname_cache], [
    AS_VAR_COPY([dpkg_varname_save], [dpkg_varname])
    AS_VAR_SET([dpkg_varname], ["-Werror dpkg_check_flag"])
    AC_COMPILE_IFELSE([
      AC_LANG_SOURCE([[]])
    ], [
      AS_VAR_SET([dpkg_varname_cache], [yes])
    ], [
      AS_VAR_SET([dpkg_varname_cache], [no])
    ])
    AS_VAR_COPY([dpkg_varname], [dpkg_varname_save])
  ])
  AS_VAR_IF([dpkg_varname_cache], [yes], [
    AS_VAR_APPEND([dpkg_varname_export], [" $1"])
  ])
  AS_VAR_POPDEF([dpkg_varname_cache])
])

# DPKG_CHECK_COMPILER_DIALECT
# ---------------------------
# Add configure option to control the compiler language dialect to use.
AC_DEFUN([DPKG_CHECK_COMPILER_DIALECT], [
  DPKG_CHECK_COMPILER_FLAG([-fstrict-flex-arrays=3])
])

# DPKG_COMPILER_DIALECT
# ---------------------
# Add configure option to enable compiler language dialect support options.
AC_DEFUN([DPKG_COMPILER_DIALECT], [
  AC_ARG_ENABLE([compiler-dialect],
    [AS_HELP_STRING([--disable-compiler-dialect],
      [Disable (detected) compiler dialect support])],
    [], [enable_compiler_dialect=yes])

  AS_IF([test "$enable_compiler_dialect" = "yes"], [
    DPKG_CHECK_COMPILER_DIALECT
    AC_LANG_PUSH([C++])
    DPKG_CHECK_COMPILER_DIALECT
    AC_LANG_POP([C++])

    CFLAGS="$DPKG_COMPILER_CFLAGS $CFLAGS"
    CXXFLAGS="$DPKG_COMPILER_CXXFLAGS $CXXFLAGS"
  ])
])

# DPKG_CHECK_COMPILER_HARDENING
# -----------------------------
# Add configure option to control the compiler hardening support.
AC_DEFUN([DPKG_CHECK_COMPILER_HARDENING], [
  DPKG_CHECK_COMPILER_FLAG([-fcf-protection=full])
  DPKG_CHECK_COMPILER_FLAG([-fstack-clash-protection])
  DPKG_CHECK_COMPILER_FLAG([-fstack-protector-strong])
  DPKG_CHECK_COMPILER_FLAG([-mbranch-protection=standard])
])

# DPKG_COMPILER_HARDENING
# -----------------------
# Add configure option to enable compiler hardening support options.
AC_DEFUN([DPKG_COMPILER_HARDENING], [
  AC_ARG_ENABLE([compiler-hardening],
    [AS_HELP_STRING([--disable-compiler-hardening],
      [Disable (detected) compiler hardening])],
    [], [enable_compiler_hardening=yes])

  AS_IF([test "$enable_compiler_hardening" = "yes"], [
    DPKG_CHECK_COMPILER_HARDENING
    AC_LANG_PUSH([C++])
    DPKG_CHECK_COMPILER_HARDENING
    AC_LANG_POP([C++])

    CFLAGS="$DPKG_COMPILER_CFLAGS $CFLAGS"
    CXXFLAGS="$DPKG_COMPILER_CXXFLAGS $CXXFLAGS"
  ])
])

# DPKG_CHECK_COMPILER_WARNINGS
# ----------------------------
# Add configure option to disable additional compiler warnings.
AC_DEFUN([DPKG_CHECK_COMPILER_WARNINGS], [
  DPKG_CHECK_COMPILER_FLAG([-Wall])
  DPKG_CHECK_COMPILER_FLAG([-Wextra])

  DPKG_CHECK_COMPILER_FLAG([-Walloca])
  DPKG_CHECK_COMPILER_FLAG([-Walloc-zero])
  DPKG_CHECK_COMPILER_FLAG([-Warray-bounds=3])
  DPKG_CHECK_COMPILER_FLAG([-Warray-bounds-pointer-arithmetic])
  DPKG_CHECK_COMPILER_FLAG([-Wassign-enum])
  DPKG_CHECK_COMPILER_FLAG([-Wbitfield-enum-conversion])
  DPKG_CHECK_COMPILER_FLAG([-Wcast-align])
  DPKG_CHECK_COMPILER_FLAG([-Wconditional-uninitialized])
  DPKG_CHECK_COMPILER_FLAG([-Wdate-time])
  DPKG_CHECK_COMPILER_FLAG([-Wdocumentation])
  DPKG_CHECK_COMPILER_FLAG([-Wduplicate-enum])
  DPKG_CHECK_COMPILER_FLAG([-Wduplicated-branches])
  DPKG_CHECK_COMPILER_FLAG([-Wduplicated-cond])
  DPKG_CHECK_COMPILER_FLAG([-Wextra-semi])
  DPKG_CHECK_COMPILER_FLAG([-Wflexible-array-extensions])
  DPKG_CHECK_COMPILER_FLAG([-Wfloat-conversion])
  DPKG_CHECK_COMPILER_FLAG([-Wfloat-equal])
  DPKG_CHECK_COMPILER_FLAG([-Wformat -Wformat-security])
  DPKG_CHECK_COMPILER_FLAG([-Wformat=2])
  DPKG_CHECK_COMPILER_FLAG([-Winit-self])
  DPKG_CHECK_COMPILER_FLAG([-Wlogical-not-parentheses])
  DPKG_CHECK_COMPILER_FLAG([-Wlogical-op])
  DPKG_CHECK_COMPILER_FLAG([-Wmissing-declarations])
  DPKG_CHECK_COMPILER_FLAG([-Wmissing-format-attribute])
  DPKG_CHECK_COMPILER_FLAG([-Wmissing-variable-declarations])
  DPKG_CHECK_COMPILER_FLAG([-Wnewline-eof])
  DPKG_CHECK_COMPILER_FLAG([-Wno-missing-field-initializers])
  DPKG_CHECK_COMPILER_FLAG([-Wno-nonnull-compare])
  DPKG_CHECK_COMPILER_FLAG([-Wno-tautological-constant-out-of-range-compare])
  DPKG_CHECK_COMPILER_FLAG([-Wno-unused-parameter])
  DPKG_CHECK_COMPILER_FLAG([-Wnull-dereference])
  DPKG_CHECK_COMPILER_FLAG([-Wpointer-arith])
  DPKG_CHECK_COMPILER_FLAG([-Wredundant-decls])
  DPKG_CHECK_COMPILER_FLAG([-Wregister])
  DPKG_CHECK_COMPILER_FLAG([-Wrestrict])
  DPKG_CHECK_COMPILER_FLAG([-Wshadow])
  DPKG_CHECK_COMPILER_FLAG([-Wshift-negative-value])
  DPKG_CHECK_COMPILER_FLAG([-Wshift-overflow=2])
  DPKG_CHECK_COMPILER_FLAG([-Wshift-sign-overflow])
  DPKG_CHECK_COMPILER_FLAG([-Wsizeof-array-argument])
  DPKG_CHECK_COMPILER_FLAG([-Wstrict-overflow=2])
  DPKG_CHECK_COMPILER_FLAG([-Wstrict-flex-arrays])
  DPKG_CHECK_COMPILER_FLAG([-Wswitch-bool])
  DPKG_CHECK_COMPILER_FLAG([-Wvla])
  DPKG_CHECK_COMPILER_FLAG([-Wwrite-strings])
  DPKG_CHECK_COMPILER_FLAG([-Wxor-used-as-pow])

  AC_LANG_CASE(
  [C], [
    DPKG_CHECK_COMPILER_FLAG([-Wbad-function-cast])
    DPKG_CHECK_COMPILER_FLAG([-Wc99-c11-compat])
    DPKG_CHECK_COMPILER_FLAG([-Wc99-compat])
    DPKG_CHECK_COMPILER_FLAG([-Wc11-extensions])
    DPKG_CHECK_COMPILER_FLAG([-Wc2x-compat])
    DPKG_CHECK_COMPILER_FLAG([-Wc2x-extensions])
    DPKG_CHECK_COMPILER_FLAG([-Wpre-c2x-compat])
    DPKG_CHECK_COMPILER_FLAG([-Wdeclaration-after-statement])
    DPKG_CHECK_COMPILER_FLAG([-Wenum-int-mismatch])
    DPKG_CHECK_COMPILER_FLAG([-Wmissing-prototypes])
    DPKG_CHECK_COMPILER_FLAG([-Wnested-externs])
    DPKG_CHECK_COMPILER_FLAG([-Wold-style-definition])
    DPKG_CHECK_COMPILER_FLAG([-Wstrict-prototypes])
  ],
  [C++], [
    AS_IF([test "$dpkg_cxx_std_version" -eq "_DPKG_CXX_CXX11_VERSION"], [
      DPKG_CHECK_COMPILER_FLAG([-Wc++11-compat])
      DPKG_CHECK_COMPILER_FLAG([-Wc++11-compat-pedantic])
    ], [test "$dpkg_cxx_std_version" -ge "_DPKG_CXX_CXX14_VERSION"], [
      DPKG_CHECK_COMPILER_FLAG([-Wc++14-compat])
      DPKG_CHECK_COMPILER_FLAG([-Wc++14-compat-pedantic])
    ])
    AS_IF([test "$dpkg_cxx_std_version" -le "_DPKG_CXX_CXX11_VERSION"], [
      DPKG_CHECK_COMPILER_FLAG([-Wc++14-extensions])
    ])
    AS_IF([test "$dpkg_cxx_std_version" -le "_DPKG_CXX_CXX14_VERSION"], [
      DPKG_CHECK_COMPILER_FLAG([-Wc++17-extensions])
    ])
    AS_IF([test "$dpkg_cxx_std_version" -le "_DPKG_CXX_CXX17_VERSION"], [
      DPKG_CHECK_COMPILER_FLAG([-Wc++20-extensions])
    ])
    AS_IF([test "$dpkg_cxx_std_version" -le "_DPKG_CXX_CXX20_VERSION"], [
      DPKG_CHECK_COMPILER_FLAG([-Wc++23-extensions])
    ])
    AS_IF([test "$dpkg_cxx_std_version" -le "_DPKG_CXX_CXX23_VERSION"], [
      DPKG_CHECK_COMPILER_FLAG([-Wc++26-extensions])
    ])
    DPKG_CHECK_COMPILER_FLAG([-Wcast-qual])
    DPKG_CHECK_COMPILER_FLAG([-Wold-style-cast])
    AS_IF([test "$dpkg_cxx_std_version" -ge "_DPKG_CXX_CXX11_VERSION"], [
      DPKG_CHECK_COMPILER_FLAG([-Wzero-as-null-pointer-constant])
    ])
  ])
])

# DPKG_COMPILER_WARNINGS
# ----------------------
# Add configure option to disable additional compiler warnings.
AC_DEFUN([DPKG_COMPILER_WARNINGS], [
  AC_ARG_ENABLE([compiler-warnings],
    [AS_HELP_STRING([--disable-compiler-warnings],
      [Disable (detected) additional compiler warnings])],
    [], [enable_compiler_warnings=yes])

  AS_IF([test "$enable_compiler_warnings" = "yes"], [
    DPKG_CHECK_COMPILER_WARNINGS
    AC_LANG_PUSH([C++])
    DPKG_CHECK_COMPILER_WARNINGS
    AC_LANG_POP([C++])

    CFLAGS="$DPKG_COMPILER_CFLAGS $CFLAGS"
    CXXFLAGS="$DPKG_COMPILER_CXXFLAGS $CXXFLAGS"
  ])
])

# DPKG_CHECK_COMPILER_SANITIZER
# -----------------------------
# Check whether the compiler sanitizer options are supported.
AC_DEFUN([DPKG_CHECK_COMPILER_SANITIZER], [
  DPKG_CHECK_COMPILER_FLAG([-fno-omit-frame-pointer])
  DPKG_CHECK_COMPILER_FLAG([-fsanitize=address])
  DPKG_CHECK_COMPILER_FLAG([-fsanitize=undefined])
])

# DPKG_COMPILER_SANITIZER
# -----------------------
# Add configure option to enable compiler sanitizer support options.
AC_DEFUN([DPKG_COMPILER_SANITIZER], [
  AC_ARG_ENABLE([compiler-sanitizer],
    [AS_HELP_STRING([--enable-compiler-sanitizer],
      [Enable compiler sanitizer support])],
    [], [enable_compiler_sanitizer=no])

  AS_IF([test "$enable_compiler_sanitizer" = "yes"], [
    DPKG_CHECK_COMPILER_SANITIZER
    AC_LANG_PUSH([C++])
    DPKG_CHECK_COMPILER_SANITIZER
    AC_LANG_POP([C++])

    LDFLAGS="$DPKG_COMPILER_CFLAGS $LDFLAGS"
    CFLAGS="$DPKG_COMPILER_CFLAGS $CFLAGS"
    CXXFLAGS="$DPKG_COMPILER_CXXFLAGS $CXXFLAGS"
  ])
])

# DPKG_COMPILER_ANALYZER
# ----------------------
# Add configure option to enable compiler analyzer support options.
# Note: This is only intended for development use, as enabling this option
# unconditionally can generate large amounts of false positives that
# require sentient triage intervention.
AC_DEFUN([DPKG_COMPILER_ANALYZER], [
  AC_ARG_ENABLE([compiler-analyzer],
    [AS_HELP_STRING([--enable-compiler-analyzer],
      [Enable compiler analyzer support])],
    [], [enable_compiler_analyzer=no])

  AS_IF([test "$enable_compiler_analyzer" = "yes"], [
    DPKG_CHECK_COMPILER_FLAG([-fanalyzer])
    AC_LANG_PUSH([C++])
    DPKG_CHECK_COMPILER_FLAG([-fanalyzer])
    AC_LANG_POP([C++])

    LDFLAGS="$COMPILER_CFLAGS $LDFLAGS"
    CFLAGS="$COMPILER_CFLAGS $CFLAGS"
    CXXFLAGS="$COMPILER_CXXFLAGS $CXXFLAGS"
  ])
])

# DPKG_COMPILER_OPTIMIZATIONS
# ---------------------------
# Add configure option to disable optimizations.
AC_DEFUN([DPKG_COMPILER_OPTIMIZATIONS], [
  AC_ARG_ENABLE([compiler-optimizations],
    [AS_HELP_STRING([--disable-compiler-optimizations],
      [Disable (detected) compiler optimizations])],
    [], [enable_compiler_optimizations=yes])

  AS_IF([test "$enable_compiler_optimizations" = "no"], [
    CFLAGS=$(echo "$CFLAGS" | $SED -e "s/ -O[[1-9]]*\b/ -O0/g")
  ])
])

# _DPKG_C_C99_VERSION
# -------------------
m4_define([_DPKG_C_C99_VERSION], [199901])

# _DPKG_C_C99_PROLOGUE
# -------------------
m4_define([_DPKG_C_C99_PROLOGUE], [[
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>

/* Variadic macro arguments. */
#define variadic_macro(foo, ...) printf(foo, __VA_ARGS__)
]])

# _DPKG_C_C99_BODY
# ----------------
m4_define([_DPKG_C_C99_BODY], [[
	int rc;

	/* Designated initializers. */
	struct { int a, b; } foo = { .a = 1, .b = 2 };

	/* Compound literals. */
	struct point { int x, y; } p = (struct point){ .x = 0, .y = 1 };
	p = (struct point){ .x = 2, .y = 4 };

	/* Trailing comma in enum. */
	enum { FIRST, SECOND, } quux;

	/* Boolean type. */
	bool bar = false;

	/* Specific size type. */
	uint32_t baz = 0;
	size_t size = SIZE_MAX;
	intmax_t imax = INTMAX_MAX;

	/* Format modifiers. */
	rc = printf("%jd", imax);
	if (rc == 3)
		return 1;
	rc = printf("%zu", size);
	if (rc == 3)
		return 1;

	/* Magic __func__ variable. */
	printf("%s", __func__);
]])

# _DPKG_C_C99_OPTS
# ----------------
m4_define([_DPKG_C_C99_OPTS], [
  -std=gnu99
  -std=c99
  -c99
  -AC99
  -xc99=all
  -qlanglvl=extc99
])

# _DPKG_C_STD_VERSION
# -------------------
m4_define([_DPKG_C_STD_VERSION], [[
#if __STDC_VERSION__ < ]]_DPKG_C_C$1_VERSION[[L
#error "Requires C$1"
#endif
]])

# _DPKG_C_STD_TRY([C-VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# ---------------
# Try compiling some C C-VERSION code to see whether it works.
AC_DEFUN([_DPKG_C_STD_TRY], [
  AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM([
      _DPKG_C_C$1_PROLOGUE
      _DPKG_C_STD_VERSION($1)
    ], [
      _DPKG_C_C$1_BODY
    ])
  ], [$2], [$3])dnl
])

# DPKG_C_STD([C-VERSION])
# ----------
# Check whether the compiler can do C C-VERSION.
#
# This is a strict requirement, as the C codebase always needs to be built.
#
# Currently supported: C99.
AC_DEFUN([DPKG_C_STD], [
  AC_CACHE_CHECK([whether $CC supports C$1], [dpkg_cv_c_std], [
    _DPKG_C_STD_TRY([$1], [dpkg_cv_c_std=yes], [dpkg_cv_c_std=no])
  ])
  AS_IF([test "$dpkg_cv_c_std" != "yes"], [
    AC_CACHE_CHECK([for $CC option to accept C$1], [dpkg_cv_c_std_opt], [
      dpkg_cv_c_std_opt=none
      dpkg_save_CC="$CC"
      for opt in m4_normalize(m4_defn([_DPKG_C_C$1_OPTS])); do
        CC="$dpkg_save_CC $opt"
        _DPKG_C_STD_TRY([$1], [dpkg_opt_worked=yes], [dpkg_opt_worked=no])
        CC="$dpkg_save_CC"

        AS_IF([test "$dpkg_opt_worked" = "yes"], [
          dpkg_cv_c_std_opt="$opt"; break
        ])
      done
    ])
    AS_IF([test "$dpkg_cv_c_std_opt" != "none"], [
      CC="$CC $dpkg_cv_c_std_opt"
      dpkg_cv_c_std=yes
    ])
  ])
  AS_IF([test "$dpkg_cv_c_std" = "yes"], [
    dpkg_c_std_version="_DPKG_C_C$1_VERSION"
  ], [
    AC_MSG_ERROR([unsupported required C$1])
  ])
])

# _DPKG_CXX_CXX11_VERSION
# -----------------------
m4_define([_DPKG_CXX_CXX11_VERSION], [201103])

# _DPKG_CXX_CXX11_PROLOGUE
# ------------------------
m4_define([_DPKG_CXX_CXX11_PROLOGUE], [[]])

# _DPKG_CXX_CXX11_BODY
# --------------------
m4_define([_DPKG_CXX_CXX11_BODY], [[
	// Null pointer keyword.
	void *ptr = nullptr;

	// Function name.
	const char *func_name = __func__;
]])

# _DPKG_CXX_CXX11_OPTS
# --------------------
m4_define([_DPKG_CXX_CXX11_OPTS], [
  -std=gnu++11
  -std=c++11
])

# _DPKG_CXX_CXX14_VERSION
# -----------------------
m4_define([_DPKG_CXX_CXX14_VERSION], [201402])

# _DPKG_CXX_CXX14_PROLOGUE
# ------------------------
m4_define([_DPKG_CXX_CXX14_PROLOGUE], [[]])

# _DPKG_CXX_CXX14_BODY
# --------------------
m4_define([_DPKG_CXX_CXX14_BODY], [
  _DPKG_CXX_CXX11_BODY
])

# _DPKG_CXX_CXX14_OPTS
# --------------------
# Define the options to try for C++14.
m4_define([_DPKG_CXX_CXX14_OPTS], [
  -std=gnu++14
  -std=c++14
])

# _DPKG_CXX_CXX17_VERSION
# -----------------------
m4_define([_DPKG_CXX_CXX17_VERSION], [201703])

# _DPKG_CXX_CXX20_VERSION
# -----------------------
m4_define([_DPKG_CXX_CXX20_VERSION], [202002])

# _DPKG_CXX_CXX23_VERSION
# -----------------------
m4_define([_DPKG_CXX_CXX23_VERSION], [202302])

# _DPKG_CXX_STD_VERSION
# ---------------------
m4_define([_DPKG_CXX_STD_VERSION], [[
#if __cplusplus < ]]_DPKG_CXX_CXX$1_VERSION[[L
#error "Requires C++$1"
#endif
]])

# _DPKG_CXX_STD_TRY([CXX-VERSION], [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND])
# -----------------
# Try compiling some C++ CXX-VERSION code to see whether it works.
AC_DEFUN([_DPKG_CXX_STD_TRY], [
  AC_LANG_PUSH([C++])
  AC_COMPILE_IFELSE([
    AC_LANG_PROGRAM([
      _DPKG_CXX_CXX$1_PROLOGUE
      _DPKG_CXX_STD_VERSION($1)
    ], [
      _DPKG_CXX_CXX$1_BODY
    ])
  ], [$2], [$3])
  AC_LANG_POP([C++])dnl
])

# DPKG_CXX_STD([CXX-VERSION])
# ------------
# Check whether the compiler can do C++ CXX-VERSION.
#
# This is a strict requirement, because even if the C++ codebase can be
# disabled with a configure option, we are still distributing C++ headers
# that we need to support and test during build.
#
# Currently supported: C++11, C++14.
AC_DEFUN([DPKG_CXX_STD], [
  AC_CACHE_CHECK([whether $CXX supports C++$1], [dpkg_cv_cxx_std], [
    _DPKG_CXX_STD_TRY([$1], [dpkg_cv_cxx_std=yes], [dpkg_cv_cxx_std=no])
  ])
  AS_IF([test "$dpkg_cv_cxx_std" != "yes"], [
    AC_CACHE_CHECK([for $CXX option to accept C++$1], [dpkg_cv_cxx_std_opt], [
      dpkg_cv_cxx_std_opt=none
      dpkg_save_CXX="$CXX"
      for opt in m4_normalize(m4_defn([_DPKG_CXX_CXX$1_OPTS])); do
        CXX="$dpkg_save_CXX $opt"
        _DPKG_CXX_STD_TRY([$1], [dpkg_opt_worked=yes], [dpkg_opt_worked=no])
        CXX="$dpkg_save_CXX"

        AS_IF([test "$dpkg_opt_worked" = "yes"], [
          dpkg_cv_cxx_std_opt="$opt"; break
        ])
      done
    ])
    AS_IF([test "$dpkg_cv_cxx_std_opt" != "none"], [
      CXX="$CXX $dpkg_cv_cxx_std_opt"
      dpkg_cv_cxx_std=yes
    ])
  ])
  AS_IF([test "$dpkg_cv_cxx_std" = "yes"], [
    dpkg_cxx_std_version="_DPKG_CXX_CXX$1_VERSION"
  ], [
    AC_MSG_ERROR([unsupported required C++$1])
  ])
])
