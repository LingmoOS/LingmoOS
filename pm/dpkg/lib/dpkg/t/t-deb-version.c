/*
 * libdpkg - Debian packaging suite library routines
 * t-deb-version.c - test deb version handling
 *
 * Copyright © 2012 Guillem Jover <guillem@debian.org>
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

#include <limits.h>
#include <stdio.h>

#include <dpkg/test.h>
#include <dpkg/deb-version.h>

static void
test_deb_version_parse(void)
{
	struct deb_version v;
	char *vs;

	/* Test valid versions. */
	test_pass(deb_version_parse(&v, "0.0") == NULL);
	test_pass(v.major == 0 && v.minor == 0);

	test_pass(deb_version_parse(&v, "1.1") == NULL);
	test_pass(v.major == 1 && v.minor == 1);

	test_pass(deb_version_parse(&v, "1.001") == NULL);
	test_pass(v.major == 1 && v.minor == 1);

	test_pass(deb_version_parse(&v, "1.0010") == NULL);
	test_pass(v.major == 1 && v.minor == 10);

	test_pass(deb_version_parse(&v, "0.939000") == NULL);
	test_pass(v.major == 0 && v.minor == 939000);

	test_pass(deb_version_parse(&v, "1.1\n") == NULL);
	test_pass(v.major == 1 && v.minor == 1);

	/* Test invalid versions. */
	test_fail(deb_version_parse(&v, "0") == NULL);
	test_fail(deb_version_parse(&v, "a") == NULL);
	test_fail(deb_version_parse(&v, "a.b") == NULL);
	test_fail(deb_version_parse(&v, "a~b") == NULL);
	test_fail(deb_version_parse(&v, " 1.1") == NULL);
	test_fail(deb_version_parse(&v, "2 .2") == NULL);
	test_fail(deb_version_parse(&v, "3. 3") == NULL);
	test_fail(deb_version_parse(&v, "4.4 ") == NULL);
	test_fail(deb_version_parse(&v, " 5.5 ") == NULL);

	/* Test integer limits. */
	if (asprintf(&vs, "%d.0", INT_MAX) < 0)
		test_bail("cannot allocate memory for asprintf()");
	test_pass(deb_version_parse(&v, vs) == NULL);
	free(vs);

	if (asprintf(&vs, "%d.0", INT_MAX - 1) < 0)
		test_bail("cannot allocate memory for asprintf()");
	test_pass(deb_version_parse(&v, vs) == NULL);
	free(vs);

	if (asprintf(&vs, "%u.0", 1U + (unsigned int)INT_MAX) < 0)
		test_bail("cannot allocate memory for asprintf()");
	test_fail(deb_version_parse(&v, vs) == NULL);
	free(vs);

	/* TODO: Complete. */
}

TEST_ENTRY(test)
{
	test_plan(24);

	test_deb_version_parse();
}
