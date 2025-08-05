/*
 * libdpkg - Debian packaging suite library routines
 * t-ar.c - test ar implementation
 *
 * Copyright © 2010 Guillem Jover <guillem@debian.org>
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

#include <string.h>

#include <dpkg/test.h>
#include <dpkg/ar.h>

static void
test_ar_normalize_name(void)
{
	struct dpkg_ar_hdr arh;

	memccpy(arh.ar_name, "member-name/    ", '\0', sizeof(arh.ar_name));
	dpkg_ar_normalize_name(&arh);
	test_str(arh.ar_name, ==, "member-name");

	memccpy(arh.ar_name, "member-name     ", '\0', sizeof(arh.ar_name));
	dpkg_ar_normalize_name(&arh);
	test_str(arh.ar_name, ==, "member-name");
}

static void
test_ar_member_is_illegal(void)
{
	struct dpkg_ar_hdr arh;

	memset(&arh, ' ', sizeof(arh));
	test_pass(dpkg_ar_member_is_illegal(&arh));

	memcpy(arh.ar_fmag, DPKG_AR_FMAG, sizeof(arh.ar_fmag));
	test_fail(dpkg_ar_member_is_illegal(&arh));
}

TEST_ENTRY(test)
{
	test_plan(4);

	test_ar_normalize_name();
	test_ar_member_is_illegal();
}
