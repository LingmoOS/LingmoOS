/*
 * libdpkg - Debian packaging suite library routines
 * t-error.c - test error message reporting
 *
 * Copyright © 2014 Guillem Jover <guillem@debian.org>
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

#include <config.h>
#include <compat.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <dpkg/test.h>
#include <dpkg/error.h>

static void
test_dpkg_error_put(void)
{
	struct dpkg_error err = DPKG_ERROR_INIT;
	char *errstr_ref = NULL;

	test_pass(err.type == DPKG_MSG_NONE);
	test_pass(err.str == NULL);

	test_pass(dpkg_put_warn(NULL, "void error") < 0);
	test_pass(dpkg_put_error(NULL, "void error") < 0);
	test_pass(dpkg_put_errno(NULL, "void error") < 0);

	test_pass(dpkg_put_warn(&err, "test warning %d", 10) < 0);
	test_pass(err.syserrno == 0);
	test_str(err.str, ==, "test warning 10");
	test_warn(err);

	test_pass(dpkg_put_error(&err, "test error %d", 20) < 0);
	test_pass(err.syserrno == 0);
	test_str(err.str, ==, "test error 20");
	test_error(err);

	errno = ENOENT;
	if (asprintf(&errstr_ref, "test errno 30 (%s)", strerror(errno)) < 0)
		test_bail("cannot allocate string");
	test_pass(dpkg_put_errno(&err, "test errno %d", 30) < 0);
	test_str(err.str, ==, errstr_ref);
	test_pass(err.syserrno == ENOENT);
	test_error(err);
	free(errstr_ref);
	errno = 0;
}

static void
test_dpkg_error_destroy(void)
{
	struct dpkg_error err = DPKG_ERROR_INIT;

	errno = ENOENT;
	test_pass(dpkg_put_errno(&err, "test destroy") < 0);
	test_pass(err.syserrno == ENOENT);
	test_pass(err.type == DPKG_MSG_ERROR);
	test_pass(err.str != NULL);
	dpkg_error_destroy(&err);
	test_pass(err.type == DPKG_MSG_NONE);
	test_pass(err.syserrno == 0);
	test_pass(err.str == NULL);
}

TEST_ENTRY(test)
{
	test_plan(24);

	test_dpkg_error_put();
	test_dpkg_error_destroy();
}
