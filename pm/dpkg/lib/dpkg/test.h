/*
 * libdpkg - Debian packaging suite library routines
 * test.h - test suite support
 *
 * Copyright © 2009-2014 Guillem Jover <guillem@debian.org>
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

#ifndef LIBDPKG_TEST_H
#define LIBDPKG_TEST_H

#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/macros.h>
#ifndef TEST_MAIN_CTOR
#include <dpkg/ehandle.h>
#define TEST_MAIN_CTOR push_error_context()
#define TEST_MAIN_DTOR pop_error_context(ehflag_normaltidy)
#endif

DPKG_BEGIN_DECLS

/**
 * @defgroup dpkg_test Test suite support
 * @ingroup dpkg-internal
 * @{
 */

#define test_bail(reason) \
	do { \
		printf("Bail out! %s\n", (reason)); \
		exit(255); \
	} while (0)

#define test_xstringify(str) \
	#str
#define test_stringify(str) \
	test_xstringify(str)

static inline void *
test_alloc(void *ptr, const char *reason)
{
	if (ptr == DPKG_NULL)
		test_bail(reason);
	return ptr;
}

#define test_alloc(ptr) \
	test_alloc((ptr), "cannot allocate memory for " #ptr " in " __FILE__ ":" test_stringify(__LINE__))

#define test_try(jmp) \
	push_error_context_jump(&(jmp), DPKG_NULL, "test try"); \
	if (!setjmp((jmp)))
#define test_catch \
	else
#define test_finally \
	pop_error_context(ehflag_normaltidy);

static inline const char *
test_get_envdir(const char *envvar)
{
	const char *envdir = getenv(envvar);
	return envdir ? envdir : ".";
}

#define test_get_srcdir() \
	test_get_envdir("srcdir")
#define test_get_builddir() \
	test_get_envdir("builddir")

static inline char *
test_data_file(const char *filename)
{
	char *pathname;
	int rc;

	rc = asprintf(&pathname, "%s/t/data/%s", test_get_srcdir(), filename);
	if (rc < 0)
		test_bail("cannot allocate data filename");

	return pathname;
}

static inline bool
test_is_verbose(void)
{
	const char *verbose = getenv("TEST_VERBOSE");
	return verbose != DPKG_NULL && strcmp(verbose, "1") == 0;
}

#ifndef TEST_OMIT_VARIABLES
static int test_id = 1;
static int test_skip_code;
static const char *test_skip_prefix;
static const char *test_skip_reason;
#endif

#define test_plan(n) \
	printf("1..%d\n", n);

#define test_skip_all(reason) \
	do { \
		printf("1..0 # SKIP %s\n", (reason)); \
		exit(0); \
	} while (0)
#define test_skip(reason) \
	printf("ok %d # SKIP %s\n", test_id++, (reason))
#define test_skip_block(cond) \
	for (test_skip_prefix = " # SKIP ", \
	     test_skip_reason = cond ? #cond : DPKG_NULL, \
	     test_skip_code = 1; \
	     test_skip_prefix; \
	     test_skip_prefix = test_skip_reason = DPKG_NULL, \
	     test_skip_code = 0)

#define test_todo(a, reason, desc) \
	do { \
		test_skip_prefix = " # TODO "; \
		test_skip_reason = reason; \
		test_case(a, "%s", desc); \
		test_skip_prefix = test_skip_reason = DPKG_NULL; \
	} while(0)
#define test_todo_block(reason) \
	for (test_skip_prefix = " # TODO ", test_skip_reason = reason; \
	     test_skip_prefix; \
	     test_skip_prefix = test_skip_reason = DPKG_NULL)

#define test_case(a, fmt, ...) \
	printf("%sok %d - " fmt "%s%s\n", \
	       test_skip_code || (a) ? "" : "not ", \
	       test_id++, __VA_ARGS__, \
	       test_skip_reason ? test_skip_prefix : "", \
	       test_skip_reason ? test_skip_reason : "")

#define test_pass(a) \
	test_case((a), "pass %s", #a)
#define test_fail(a) \
	test_case(!(a), "fail %s", #a)
#define test_str(a, op, b) \
	test_case(strcmp((a), (b)) op 0, "strcmp '%s' %s '%s'", a, #op, b)
#define test_mem(a, op, b, size) \
	test_case(memcmp((a), (b), (size)) op 0, "memcmp %p %s %p", a, #op, b)

/* Specific test macros. */
#define test_warn(e) \
	do { \
		test_pass((e).type == DPKG_MSG_WARN); \
		dpkg_error_destroy(&(e)); \
	} while (0)
#define test_error(e) \
	do { \
		test_pass((e).type == DPKG_MSG_ERROR); \
		dpkg_error_destroy(&(e)); \
	} while (0)

/** @} */

DPKG_END_DECLS

#define TEST_ENTRY(name) \
static void name(void); \
int \
main(int argc, char **argv) \
{ \
	setvbuf(stdout, DPKG_NULL, _IOLBF, 0); \
 \
	TEST_MAIN_CTOR; \
	name(); \
	TEST_MAIN_DTOR; \
	return 0; \
} \
static void \
name(void)

#endif
