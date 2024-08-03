dnl acinclude.m4 - configure macros used by lpm-contrib
dnl Add some custom macros for lpm-contrib

dnl CFLAGS_ADD(PARAMETER, VARIABLE)
dnl Adds parameter to VARIABLE if the compiler supports it.  For example,
dnl CFLAGS_ADD([-Wall],[WARN_FLAGS]).
AC_DEFUN([CFLAGS_ADD],
[AS_VAR_PUSHDEF([my_cflags], [cflags_cv_warn_$1])dnl
AC_CACHE_CHECK([whether compiler handles $1], [my_cflags], [
  save_CFLAGS="$CFLAGS"
  CFLAGS="${CFLAGS} -Werror=unknown-warning-option"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
                    [],
                    [CFLAGS="$save_CFLAGS"])
  CFLAGS="${CFLAGS} $1"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([])],
                    [AS_VAR_SET([my_cflags], [yes])],
                    [AS_VAR_SET([my_cflags], [no])])
  CFLAGS="$save_CFLAGS"
])
AS_VAR_PUSHDEF([new_cflags], [[$2]])dnl
AS_VAR_IF([my_cflags], [yes], [AS_VAR_APPEND([new_cflags], [" $1"])])
AS_VAR_POPDEF([new_cflags])dnl
AS_VAR_POPDEF([my_cflags])dnl
m4_ifval([$2], [AS_LITERAL_IF([$2], [AC_SUBST([$2])], [])])dnl
])
