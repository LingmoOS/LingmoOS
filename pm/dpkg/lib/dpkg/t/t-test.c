/*
 * libdpkg - Debian packaging suite library routines
 * t-test.c - test suite self tests
 *
 * Copyright © 2009, 2014 Guillem Jover <guillem@debian.org>
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

#include <dpkg/test.h>

TEST_ENTRY(test)
{
	test_plan(22);

	test_pass(1);
	test_fail(0);

	test_skip("ignore test");

	test_skip_block(1) {
		test_pass(0);
		test_pass(1);
	}

	test_todo(0, "unimplemented test", "failing test");

	test_todo_block("unimplemented block") {
		test_pass(0);
		test_fail(1);
	}

	test_str("aaa", ==, "aaa");
	test_str("aaa", <, "bbb");
	test_str("ccc", >, "bbb");
	test_str("ccc", !=, "bbb");

	test_mem("aaa", ==, "aaa", 3);
	test_mem("aaa", <, "bbb", 3);
	test_mem("ccc", >, "bbb", 3);
	test_mem("ccc", !=, "bbb", 3);

	test_mem("abcd", ==, "abcd", 4);
	test_mem("abcd", ==, "abcd", 5);
	test_mem("ababcd", ==, "ababff", 4);
	test_mem("ababcd", !=, "ababff", 6);

	setenv("srcdir", "aaa", 1);
	setenv("builddir", "bbb", 1);
	test_str(test_get_srcdir(), ==, "aaa");
	test_str(test_get_builddir(), ==, "bbb");
}
