/*
 * libdpkg - Debian packaging suite library routines
 * t-macros.c - test C support macros
 *
 * Copyright © 2009,2012 Guillem Jover <guillem@debian.org>
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
#include <dpkg/macros.h>

TEST_ENTRY(test)
{
	test_plan(12);

	test_pass(min(10, 30) == 10);
	test_pass(min(30, 10) == 10);
	test_pass(min(0, 10) == 0);
	test_pass(min(-10, 0) == -10);

	test_pass(max(10, 30) == 30);
	test_pass(max(30, 10) == 30);
	test_pass(max(0, 10) == 10);
	test_pass(max(-10, 0) == 0);

	test_pass(clamp(0, 0, 0) == 0);
	test_pass(clamp(0, -10, 10) == 0);
	test_pass(clamp(20, -10, 10) == 10);
	test_pass(clamp(-20, -10, 10) == -10);
}
