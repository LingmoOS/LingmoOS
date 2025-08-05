/*
 * libdpkg - Debian packaging suite library routines
 * t-arch.c - test dpkg_arch implementation
 *
 * Copyright © 2011 Linaro Limited
 * Copyright © 2011 Raphaël Hertzog <hertzog@debian.org>
 * Copyright © 2011-2014 Guillem Jover <guillem@debian.org>
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
#include <dpkg/varbuf.h>
#include <dpkg/arch.h>

static void
test_dpkg_arch_name_is_illegal(void)
{
	/* Test invalid architecture names. */
	test_fail(dpkg_arch_name_is_illegal("") == NULL);
	test_fail(dpkg_arch_name_is_illegal("-i386") == NULL);
	test_fail(dpkg_arch_name_is_illegal(" i386") == NULL);
	test_fail(dpkg_arch_name_is_illegal(":any") == NULL);
	test_fail(dpkg_arch_name_is_illegal("amd64_test") == NULL);
	test_fail(dpkg_arch_name_is_illegal("i386:test") == NULL);
	test_fail(dpkg_arch_name_is_illegal("i386 amd64") == NULL);
	test_fail(dpkg_arch_name_is_illegal("i386,amd64") == NULL);
	test_fail(dpkg_arch_name_is_illegal("i386|amd64") == NULL);

	/* Test valid architecture names. */
	test_pass(dpkg_arch_name_is_illegal("i386") == NULL);
	test_pass(dpkg_arch_name_is_illegal("amd64") == NULL);
	test_pass(dpkg_arch_name_is_illegal("hurd-i386") == NULL);
	test_pass(dpkg_arch_name_is_illegal("kfreebsd-i386") == NULL);
	test_pass(dpkg_arch_name_is_illegal("kfreebsd-amd64") == NULL);
	test_pass(dpkg_arch_name_is_illegal("ia64") == NULL);
	test_pass(dpkg_arch_name_is_illegal("alpha") == NULL);
	test_pass(dpkg_arch_name_is_illegal("armel") == NULL);
	test_pass(dpkg_arch_name_is_illegal("hppa") == NULL);
	test_pass(dpkg_arch_name_is_illegal("mips") == NULL);
	test_pass(dpkg_arch_name_is_illegal("mipsel") == NULL);
	test_pass(dpkg_arch_name_is_illegal("powerpc") == NULL);
	test_pass(dpkg_arch_name_is_illegal("s390") == NULL);
	test_pass(dpkg_arch_name_is_illegal("sparc") == NULL);
}

static void
test_dpkg_arch_get_list(void)
{
	struct dpkg_arch *arch;
	int count = 1;

	/* Must never return NULL. */
	arch = dpkg_arch_get_list();
	test_alloc(arch);

	while ((arch = arch->next))
		count++;

	/* The default list should contain 3 architectures. */
	test_pass(count == 3);
}

static void
test_dpkg_arch_find(void)
{
	struct dpkg_arch *arch;

	/* Test existence and initial values of default architectures. */
	arch = dpkg_arch_find("all");
	test_pass(arch->type == DPKG_ARCH_ALL);
	test_pass(dpkg_arch_get(DPKG_ARCH_ALL) == arch);
	arch = dpkg_arch_find(ARCHITECTURE);
	test_pass(arch->type == DPKG_ARCH_NATIVE);
	test_pass(dpkg_arch_get(DPKG_ARCH_NATIVE) == arch);
	arch = dpkg_arch_find("any");
	test_pass(arch->type == DPKG_ARCH_WILDCARD);
	test_pass(dpkg_arch_get(DPKG_ARCH_WILDCARD) == arch);

	/* Test missing architecture. */
	arch = dpkg_arch_find(NULL);
	test_pass(arch->type == DPKG_ARCH_NONE);
	test_pass(dpkg_arch_get(DPKG_ARCH_NONE) == arch);
	test_str(arch->name, ==, "");

	/* Test empty architectures. */
	arch = dpkg_arch_find("");
	test_pass(arch->type == DPKG_ARCH_EMPTY);
	test_pass(dpkg_arch_get(DPKG_ARCH_EMPTY) == arch);
	test_str(arch->name, ==, "");

	/* Test for an unknown type. */
	DPKG_IGNORE_WARNING_ASSIGN_ENUM();
	test_pass(dpkg_arch_get(1000) == NULL);
	DPKG_ACCEPT_WARNING_ASSIGN_ENUM();

	/* New valid architectures are marked unknown. */
	arch = dpkg_arch_find("foobar");
	test_pass(arch->type == DPKG_ARCH_UNKNOWN);
	test_str(arch->name, ==, "foobar");

	/* New illegal architectures are marked illegal. */
	arch = dpkg_arch_find("a:b");
	test_pass(arch->type == DPKG_ARCH_ILLEGAL);
	test_str(arch->name, ==, "a:b");
}

static void
test_dpkg_arch_reset_list(void)
{
	dpkg_arch_reset_list();

	test_dpkg_arch_get_list();
}

static void
test_dpkg_arch_modify(void)
{
	struct dpkg_arch *arch;

	dpkg_arch_reset_list();

	/* Insert a new unknown arch. */
	arch = dpkg_arch_find("foo");
	test_pass(arch->type == DPKG_ARCH_UNKNOWN);
	test_str(arch->name, ==, "foo");

	/* Check that existing unknown arch gets tagged. */
	arch = dpkg_arch_add("foo");
	test_pass(arch->type == DPKG_ARCH_FOREIGN);
	test_str(arch->name, ==, "foo");

	/* Check that new unknown arch gets tagged. */
	arch = dpkg_arch_add("quux");
	test_pass(arch->type == DPKG_ARCH_FOREIGN);
	test_str(arch->name, ==, "quux");

	/* Unmark foreign architectures. */

	arch = dpkg_arch_find("foo");
	dpkg_arch_unmark(arch);
	test_pass(arch->type == DPKG_ARCH_UNKNOWN);

	arch = dpkg_arch_find("bar");
	dpkg_arch_unmark(arch);
	test_pass(arch->type == DPKG_ARCH_UNKNOWN);

	arch = dpkg_arch_find("quux");
	dpkg_arch_unmark(arch);
	test_pass(arch->type == DPKG_ARCH_UNKNOWN);
}

static void
test_dpkg_arch_varbuf_archqual(void)
{
	struct varbuf vb = VARBUF_INIT;

	varbuf_add_archqual(&vb, dpkg_arch_get(DPKG_ARCH_NONE));
	test_str(varbuf_str(&vb), ==, "");
	varbuf_reset(&vb);

	varbuf_add_archqual(&vb, dpkg_arch_get(DPKG_ARCH_EMPTY));
	test_str(varbuf_str(&vb), ==, "");
	varbuf_reset(&vb);

	varbuf_add_archqual(&vb, dpkg_arch_get(DPKG_ARCH_ALL));
	test_str(varbuf_str(&vb), ==, ":all");
	varbuf_reset(&vb);

	varbuf_add_archqual(&vb, dpkg_arch_get(DPKG_ARCH_WILDCARD));
	test_str(varbuf_str(&vb), ==, ":any");
	varbuf_reset(&vb);

	varbuf_destroy(&vb);
}

static void
test_dpkg_arch_describe(void)
{
	struct dpkg_arch *arch;

	arch = dpkg_arch_get(DPKG_ARCH_NONE);
	test_str(dpkg_arch_describe(arch), ==, "<none>");

	arch = dpkg_arch_get(DPKG_ARCH_EMPTY);
	test_str(dpkg_arch_describe(arch), ==, "<empty>");

	arch = dpkg_arch_get(DPKG_ARCH_ALL);
	test_str(dpkg_arch_describe(arch), ==, "all");

	arch = dpkg_arch_get(DPKG_ARCH_WILDCARD);
	test_str(dpkg_arch_describe(arch), ==, "any");

	arch = dpkg_arch_get(DPKG_ARCH_NATIVE);
	test_str(dpkg_arch_describe(arch), ==, ARCHITECTURE);
}

TEST_ENTRY(test)
{
	test_plan(60);

	test_dpkg_arch_name_is_illegal();
	test_dpkg_arch_get_list();
	test_dpkg_arch_find();
	test_dpkg_arch_reset_list();
	test_dpkg_arch_modify();
	test_dpkg_arch_varbuf_archqual();
	test_dpkg_arch_describe();
}
