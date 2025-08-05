/*
 * libdpkg - Debian packaging suite library routines
 * t-pkg-show.c - test pkg-show implementation
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
#include <dpkg/dpkg-db.h>
#include <dpkg/arch.h>

static void
test_pkg_show_name(void)
{
	struct dpkg_arch *arch;
	struct pkginfo *pkg;
	const char *pkgname;

	arch = dpkg_arch_find("arch");
	test_pass(arch);

	pkg = pkg_hash_find_pkg("test", arch);
	test_pass(pkg);
	test_str(pkg->set->name, ==, "test");
	test_pass(pkg->installed.arch->type == DPKG_ARCH_UNKNOWN);

	pkgname = pkg_name(pkg, pnaw_never);
	test_pass(pkgname);
	test_str(pkgname, ==, "test");

	pkgname = pkg_name(pkg, pnaw_nonambig);
	test_pass(pkgname);
	test_str(pkgname, ==, "test:arch");

	pkgname = pkg_name(pkg, pnaw_always);
	test_pass(pkgname);
	test_str(pkgname, ==, "test:arch");

	pkgname = pkg_name(pkg, pnaw_same);
	test_pass(pkgname);
	test_str(pkgname, ==, "test");

	pkg->installed.multiarch = PKG_MULTIARCH_SAME;
	pkgname = pkg_name(pkg, pnaw_same);
	test_pass(pkgname);
	test_str(pkgname, ==, "test:arch");
}

TEST_ENTRY(test)
{
	test_plan(14);

	test_pkg_show_name();
}
