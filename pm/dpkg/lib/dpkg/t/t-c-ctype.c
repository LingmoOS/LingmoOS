/*
 * libdpkg - Debian packaging suite library routines
 * t-c-ctype.c - test C locale ctype functions
 *
 * Copyright © 2009-2014 Guillem Jover <guillem@debian.org>
 *
 * This is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2,
 * or (at your option) any later version.
 *
 * This is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <config.h>
#include <compat.h>

#include <dpkg/test.h>
#include <dpkg/c-ctype.h>

static void
test_ctype(void)
{
	int c;

	for (c = -1; c < 256; c++) {
		/* Test blank. */
		if (c == '\t' || c == ' ')
			test_pass(c_isblank(c));
		else
			test_fail(c_isblank(c));

		/* Test white. */
		if (c == '\t' || c == ' ' || c == '\n')
			test_pass(c_iswhite(c));
		else
			test_fail(c_iswhite(c));

		/* Test space. */
		if (c == '\t' || c == '\v' || c == '\f' ||
		    c == '\r' || c == '\n' || c == ' ')
			test_pass(c_isspace(c));
		else
			test_fail(c_isspace(c));

		/* Test digit. */
		if (c >= '0' && c <= '9')
			test_pass(c_isdigit(c));
		else
			test_fail(c_isdigit(c));

		/* Test lower case. */
		if (c >= 'a' && c <= 'z')
			test_pass(c_islower(c));
		else
			test_fail(c_islower(c));

		/* Test upper case. */
		if (c >= 'A' && c <= 'Z')
			test_pass(c_isupper(c));
		else
			test_fail(c_isupper(c));

		/* Test alpha. */
		if (c_islower(c) || c_isupper(c))
			test_pass(c_isalpha(c));
		else
			test_fail(c_isalpha(c));

		/* Test alphanumeric. */
		if (c_isdigit(c) || c_isalpha(c))
			test_pass(c_isalnum(c));
		else
			test_fail(c_isalnum(c));
	}
}

static void
test_casing(void)
{
	test_pass(c_tolower('A') == 'a');
	test_pass(c_tolower('Z') == 'z');

	test_pass(c_tolower('a') == 'a');
	test_pass(c_tolower('z') == 'z');

	test_pass(c_tolower('0') == '0');
	test_pass(c_tolower('9') == '9');

	/* Test if we can handle the value for EOF. */
	test_pass(c_tolower(-1) == -1);
}

TEST_ENTRY(test)
{
	test_plan(2063);

	test_ctype();
	test_casing();
}
