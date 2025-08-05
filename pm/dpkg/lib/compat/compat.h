/*
 * libcompat - system compatibility library
 * compat.h - system compatibility declarations
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008, 2009 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef COMPAT_H
#define COMPAT_H

#ifndef TEST_LIBCOMPAT
#define TEST_LIBCOMPAT 0
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_STRNLEN) || !defined(HAVE_STRNDUP) || \
    !defined(HAVE_C99_SNPRINTF)
#include <stddef.h>
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_ASPRINTF) || !defined(HAVE_C99_SNPRINTF)
#include <stdarg.h>
#endif

#if  TEST_LIBCOMPAT || !defined(HAVE_VA_COPY)
#include <string.h>
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_FGETPWENT) || !defined(HAVE_FGETGRENT)
#include <stdio.h>
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_FGETPWENT)
#include <pwd.h>
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_FGETGRENT)
#include <grp.h>
#endif

/* Language definitions. */

/* Supported since gcc 5.1.0 and clang 2.9.0. For attributes that appeared
 * before these versions, in addition we need to do version checks.  */
#ifndef __has_attribute
#define __has_attribute(x)	0
#endif

#ifndef __has_include
#define __has_include(x)	0
#endif

#ifdef __GNUC__
#define LIBCOMPAT_GCC_VERSION (__GNUC__ << 8 | __GNUC_MINOR__)
#else
#define LIBCOMPAT_GCC_VERSION 0
#endif

#if LIBCOMPAT_GCC_VERSION >= 0x0300 || __has_attribute(__format__)
#define LIBCOMPAT_ATTR_FMT(t, f, a)	__attribute__((__format__(t, f, a)))
#define LIBCOMPAT_ATTR_PRINTF(n)	LIBCOMPAT_ATTR_FMT(__printf__, n, n + 1)
#define LIBCOMPAT_ATTR_VPRINTF(n)	LIBCOMPAT_ATTR_FMT(__printf__, n, 0)
#else
#define LIBCOMPAT_ATTR_FMT(t, f, a)
#define LIBCOMPAT_ATTR_PRINTF(n)
#define LIBCOMPAT_ATTR_VPRINTF(n)
#endif

#if LIBCOMPAT_GCC_VERSION >= 0x0300 || __has_attribute(__noreturn__)
#define LIBCOMPAT_ATTR_NORET		__attribute__((__noreturn__))
#else
#define LIBCOMPAT_ATTR_NORET
#endif

#if LIBCOMPAT_GCC_VERSION >= 0x0400 || __has_attribute(__sentinel__)
#define LIBCOMPAT_ATTR_SENTINEL		__attribute__((__sentinel__))
#else
#define LIBCOMPAT_ATTR_SENTINEL
#endif

#if __has_attribute(__enum_extensibility__)
#define LIBCOMPAT_ATTR_ENUM_FLAGS \
	__attribute__((__enum_extensibility__(closed),__flag_enum__))
#else
#define LIBCOMPAT_ATTR_ENUM_FLAGS
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifndef HAVE_OFFSETOF
#define offsetof(st, m) ((size_t)&((st *)NULL)->m)
#endif

#ifndef HAVE_MAKEDEV
#define makedev(maj, min) ((((maj) & 0xff) << 8) | ((min) & 0xff))
#endif

#ifndef HAVE_O_NOFOLLOW
#define O_NOFOLLOW 0
#endif

#ifndef HAVE_P_TMPDIR
#define P_tmpdir "/tmp"
#endif

/*
 * Define WCOREDUMP if we don't have it already, coredumps won't be
 * detected, though.
 */
#ifndef HAVE_WCOREDUMP
#define WCOREDUMP(x) 0
#endif

#ifndef HAVE_VA_COPY
#define va_copy(dest, src) memcpy(&(dest), &(src), sizeof(va_list))
#endif

#if TEST_LIBCOMPAT
#undef snprintf
#define snprintf test_snprintf
#undef vsnprintf
#define vsnprintf test_vsnprintf
#undef asprintf
#define asprintf test_asprintf
#undef vasprintf
#define vasprintf test_vasprintf
#undef strchrnul
#define strchrnul test_strchrnul
#undef strndup
#define strndup test_strndup
#undef strnlen
#define strnlen test_strnlen
#undef strerror
#define strerror test_strerror
#undef strsignal
#define strsignal test_strsignal
#undef fgetpwent
#define fgetpwent test_fgetpwent
#undef fgetgrent
#define fgetgrent test_fgetgrent
#undef scandir
#define scandir test_scandir
#undef alphasort
#define alphasort test_alphasort
#undef unsetenv
#define unsetenv test_unsetenv
#endif

#if !HAVE_DECL_SYS_SIGLIST
extern const char *const sys_siglist[];
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_C99_SNPRINTF)
int snprintf(char *str, size_t n, char const *fmt, ...)
	LIBCOMPAT_ATTR_PRINTF(3);
int vsnprintf(char *buf, size_t maxsize, const char *fmt, va_list args)
	LIBCOMPAT_ATTR_VPRINTF(3);
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_ASPRINTF)
int asprintf(char **str, char const *fmt, ...)
	LIBCOMPAT_ATTR_PRINTF(2);
int vasprintf(char **str, const char *fmt, va_list args)
	LIBCOMPAT_ATTR_VPRINTF(2);
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_STRCHRNUL)
char *strchrnul(const char *s, int c);
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_STRNLEN)
size_t strnlen(const char *s, size_t n);
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_STRNDUP)
char *strndup(const char *s, size_t n);
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_STRERROR)
const char *strerror(int);
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_STRSIGNAL)
const char *strsignal(int);
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_FGETPWENT)
struct passwd *fgetpwent(FILE *fp);
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_FGETGRENT)
struct group *fgetgrent(FILE *fp);
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_SCANDIR)
struct dirent;
int scandir(const char *dir, struct dirent ***namelist,
            int (*filter)(const struct dirent *),
            int (*cmp)(const void *, const void *));
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_ALPHASORT)
int alphasort(const void *a, const void *b);
#endif

#if TEST_LIBCOMPAT || !defined(HAVE_UNSETENV)
int unsetenv(const char *x);
#endif

#ifdef __cplusplus
}
#endif

#endif /* COMPAT_H */
