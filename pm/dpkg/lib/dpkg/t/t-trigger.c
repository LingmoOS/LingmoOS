/*
 * libdpkg - Debian packaging suite library routines
 * t-trigger.c - test triggers
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

#include <dpkg/test.h>
#include <dpkg/triglib.h>

static void
test_trig_name_is_illegal(void)
{
	/* Test invalid trigger names. */
	test_fail(trig_name_is_illegal("") == NULL);
	test_fail(trig_name_is_illegal("\a") == NULL);
	test_fail(trig_name_is_illegal("\t") == NULL);
	test_fail(trig_name_is_illegal("\200") == NULL);
	test_fail(trig_name_is_illegal("trigger name") == NULL);

	/* Test valid trigger names. */
	test_pass(trig_name_is_illegal("TRIGGER") == NULL);
	test_pass(trig_name_is_illegal("trigger") == NULL);
	test_pass(trig_name_is_illegal("0123456789") == NULL);
	test_pass(trig_name_is_illegal("/file/trigger") == NULL);
}

TEST_ENTRY(test)
{
	test_plan(9);

	test_trig_name_is_illegal();
}
