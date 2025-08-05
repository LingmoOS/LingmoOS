/*
 * libdpkg - Debian packaging suite library routines
 * macros.h - C language support macros
 *
 * Copyright © 2008-2012 Guillem Jover <guillem@debian.org>
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

#ifndef LIBDPKG_MACROS_H
#define LIBDPKG_MACROS_H

/**
 * @defgroup macros C language support macros
 * @ingroup dpkg-public
 * @{
 */

#ifndef LIBDPKG_VOLATILE_API
#error "The libdpkg API is to be considered volatile, please read 'README.api'."
#endif

/* Language definitions. */

#ifndef __has_include
#define __has_include(x)	0
#endif

/* Supported since clang 1.0. */
#ifndef __has_warning
#define __has_warning(w) (0)
#endif

#define DPKG_PRAGMA(x) _Pragma(#x)

#if defined(__clang__)
#define DPKG_PRAGMA_CC(x) DPKG_PRAGMA(clang x)
#elif defined(__GNUC__)
#define DPKG_PRAGMA_CC(x) DPKG_PRAGMA(GCC x)
#else
#define DPKG_PRAGMA_CC(x)
#endif

#define DPKG_IGNORE_WARNING(w) \
       DPKG_PRAGMA_CC(diagnostic push); \
       DPKG_PRAGMA_CC(diagnostic ignored w)
#define DPKG_ACCEPT_WARNING() \
       DPKG_PRAGMA_CC(diagnostic pop)

#if __has_warning("-Wassign-enum")
#define DPKG_IGNORE_WARNING_ASSIGN_ENUM()	DPKG_IGNORE_WARNING("-Wassign-enum")
#define DPKG_ACCEPT_WARNING_ASSIGN_ENUM()	DPKG_ACCEPT_WARNING()
#else
#define DPKG_IGNORE_WARNING_ASSIGN_ENUM()
#define DPKG_ACCEPT_WARNING_ASSIGN_ENUM()
#endif

/* Supported since gcc 5.1.0 and clang 2.9.0. For attributes that appeared
 * before these versions, in addition we need to do version checks.  */
#ifndef __has_attribute
#define __has_attribute(x)	0
#endif

#ifdef __GNUC__
#define DPKG_GCC_VERSION (__GNUC__ << 8 | __GNUC_MINOR__)
#else
#define DPKG_GCC_VERSION 0
#endif

#if DPKG_GCC_VERSION >= 0x0300 || __has_attribute(__unused__)
#define DPKG_ATTR_UNUSED	__attribute__((__unused__))
#else
#define DPKG_ATTR_UNUSED
#endif

#if DPKG_GCC_VERSION >= 0x0300 || __has_attribute(__const__)
#define DPKG_ATTR_CONST		__attribute__((__const__))
#else
#define DPKG_ATTR_CONST
#endif

#if DPKG_GCC_VERSION >= 0x0300 || __has_attribute(__pure__)
#define DPKG_ATTR_PURE		__attribute__((__pure__))
#else
#define DPKG_ATTR_PURE
#endif

#if DPKG_GCC_VERSION >= 0x0300 || __has_attribute(__malloc__)
#define DPKG_ATTR_MALLOC	__attribute__((__malloc__))
#else
#define DPKG_ATTR_MALLOC
#endif

#if DPKG_GCC_VERSION >= 0x0300 || __has_attribute(__noreturn__)
#define DPKG_ATTR_NORET		__attribute__((__noreturn__))
#else
#define DPKG_ATTR_NORET
#endif

#if DPKG_GCC_VERSION >= 0x0300 || __has_attribute(__format__)
#define DPKG_ATTR_FMT(t, f, a)	__attribute__((__format__(t, f, a)))
#define DPKG_ATTR_PRINTF(n)	DPKG_ATTR_FMT(__printf__, n, n + 1)
#define DPKG_ATTR_VPRINTF(n)	DPKG_ATTR_FMT(__printf__, n, 0)
#else
#define DPKG_ATTR_FMT(t, f, a)
#define DPKG_ATTR_PRINTF(n)
#define DPKG_ATTR_VPRINTF(n)
#endif

#if DPKG_GCC_VERSION > 0x0302 || __has_attribute(__nonnull__)
#define DPKG_ATTR_NONNULL(...)	__attribute__((__nonnull__(__VA_ARGS__)))
#else
#define DPKG_ATTR_NONNULL(...)
#endif

#if DPKG_GCC_VERSION > 0x0302 || __has_attribute(__warn_unused_result__)
#define DPKG_ATTR_REQRET	__attribute__((__warn_unused_result__))
#else
#define DPKG_ATTR_REQRET
#endif

#if DPKG_GCC_VERSION >= 0x0400 || __has_attribute(__sentinel__)
#define DPKG_ATTR_SENTINEL	__attribute__((__sentinel__))
#else
#define DPKG_ATTR_SENTINEL
#endif

#if DPKG_GCC_VERSION >= 0x0801 || __has_attribute(__nonstring__)
#define DPKG_ATTR_NONSTRING	__attribute__((__nonstring__))
#else
#define DPKG_ATTR_NONSTRING
#endif

#if __has_attribute(__enum_extensibility__)
#define DPKG_ATTR_ENUM_FLAGS \
	__attribute__((__enum_extensibility__(closed),__flag_enum__))
#else
#define DPKG_ATTR_ENUM_FLAGS
#endif

#if defined(__cplusplus)
#define DPKG_ATTR_THROW(exception)
#define DPKG_ATTR_NOEXCEPT		noexcept
#endif

#ifdef __cplusplus
#define DPKG_BEGIN_DECLS	extern "C" {
#define DPKG_END_DECLS		}
#else
#define DPKG_BEGIN_DECLS
#define DPKG_END_DECLS
#endif

/**
 * @def DPKG_NULL
 *
 * A null pointer constant that works on C or C++.
 *
 * To be used only on header files, where having to conditionalize the code
 * to use either NULL or nullptr would be too cumbersome. Non-header files
 * should use the appropriate constant directly.
 */
#if defined(__cplusplus) || __STDC_VERSION__ > 201710L
#define DPKG_NULL      nullptr
#else
#define DPKG_NULL      NULL
#endif

/**
 * @def DPKG_STATIC_CAST
 *
 * Cast an expression to a given type that works on C or C++.
 *
 * To be used only on header files, where having to conditionalize the code
 * to use either NULL or nullptr would be too cumbersome. Non-header files
 * should use the appropriate constant directly.
 */
#if defined(__cplusplus)
#define DPKG_STATIC_CAST(type, expr) static_cast<type>(expr)
#else
#define DPKG_STATIC_CAST(type, expr) (type)(expr)
#endif

/**
 * @def DPKG_BIT
 *
 * Return the integer value of bit n.
 */
#define DPKG_BIT(n)	(1UL << (n))

/**
 * @def array_count
 *
 * Returns the amount of items in an array.
 */
#ifndef array_count
#define array_count(a) (sizeof(a) / sizeof((a)[0]))
#endif

/* For C++ use native implementations from STL or similar. */
#ifndef __cplusplus
#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif
#endif

/**
 * @def clamp
 *
 * Returns a normalized value within the low and high limits.
 *
 * @param v The value to clamp.
 * @param l The low limit.
 * @param h The high limit.
 */
/* For C++ use native implementations from STL or similar. */
#ifndef __cplusplus
#ifndef clamp
#define clamp(v, l, h) ((v) > (h) ? (h) : ((v) < (l) ? (l) : (v)))
#endif
#endif

/** @} */

#endif /* LIBDPKG_MACROS_H */
