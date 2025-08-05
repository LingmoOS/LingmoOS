/*
 * libdpkg - Debian packaging suite library routines
 * t-namevalue.c - test name/value implementation
 *
 * Copyright © 2018 Guillem Jover <guillem@debian.org>
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
#include <dpkg/namevalue.h>
#include <dpkg/dpkg-db.h>

static void
test_namevalue(void)
{
	const struct namevalue *nv;

	nv = namevalue_find_by_name(booleaninfos, "");
	test_pass(nv == NULL);

	nv = namevalue_find_by_name(booleaninfos, "no");
	test_pass(nv != NULL);
	test_pass(nv->value == false);
	test_pass(nv->length == strlen("no"));
	test_str(nv->name, ==, "no");
}

TEST_ENTRY(test)
{
	test_plan(5);

	test_namevalue();
}
