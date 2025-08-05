/*
 * libdpkg - Debian packaging suite library routines
 * t-pkg-hash.c - test pkg-hash implementation
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
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg.h>

static void
test_pkg_hash(void)
{
	struct dpkg_arch *arch;
	struct pkgset *set;
	struct pkginfo *pkg;
	struct pkg_hash_iter *iter;
	int pkginstance;

	test_pass(pkg_hash_count_set() == 0);
	test_pass(pkg_hash_count_pkg() == 0);

	set = pkg_hash_find_set("pkg-aa");
	test_pass(set != NULL);
	test_str(set->name, ==, "pkg-aa");
	test_pass(pkg_hash_count_set() == 1);
	test_pass(pkg_hash_count_pkg() == 1);

	set = pkg_hash_find_set("pkg-aa");
	test_pass(set != NULL);
	test_str(set->name, ==, "pkg-aa");
	test_pass(pkg_hash_count_set() == 1);
	test_pass(pkg_hash_count_pkg() == 1);

	set = pkg_hash_find_set("Pkg-AA");
	test_pass(set != NULL);
	test_str(set->name, ==, "pkg-aa");
	test_pass(pkg_hash_count_set() == 1);
	test_pass(pkg_hash_count_pkg() == 1);

	set = pkg_hash_find_set("pkg-bb");
	test_pass(set != NULL);
	pkg_set_status(&set->pkg, PKG_STAT_INSTALLED);
	test_str(set->name, ==, "pkg-bb");
	test_pass(pkg_hash_count_set() == 2);
	test_pass(pkg_hash_count_pkg() == 2);

	set = pkg_hash_find_set("pkg-cc");
	test_pass(set != NULL);
	test_str(set->name, ==, "pkg-cc");
	test_pass(pkg_hash_count_set() == 3);
	test_pass(pkg_hash_count_pkg() == 3);

	arch = dpkg_arch_find("arch-xx");
	pkg = pkg_hash_find_pkg("pkg-aa", arch);
	test_pass(pkg != NULL);
	pkg_set_status(pkg, PKG_STAT_INSTALLED);
	test_str(pkg->set->name, ==, "pkg-aa");
	test_str(pkg->installed.arch->name, ==, "arch-xx");
	test_str(pkg->available.arch->name, ==, "arch-xx");
	test_pass(pkg_hash_count_set() == 3);
	test_pass(pkg_hash_count_pkg() == 3);

	arch = dpkg_arch_find("arch-yy");
	pkg = pkg_hash_find_pkg("pkg-aa", arch);
	test_pass(pkg != NULL);
	test_str(pkg->set->name, ==, "pkg-aa");
	test_str(pkg->installed.arch->name, ==, "arch-yy");
	test_str(pkg->available.arch->name, ==, "arch-yy");
	test_pass(pkg_hash_count_set() == 3);
	test_pass(pkg_hash_count_pkg() == 4);

	arch = dpkg_arch_find("arch-zz");
	pkg = pkg_hash_find_pkg("pkg-aa", arch);
	test_pass(pkg != NULL);
	pkg_set_status(pkg, PKG_STAT_UNPACKED);
	test_str(pkg->set->name, ==, "pkg-aa");
	test_str(pkg->installed.arch->name, ==, "arch-zz");
	test_str(pkg->available.arch->name, ==, "arch-zz");
	test_pass(pkg_hash_count_set() == 3);
	test_pass(pkg_hash_count_pkg() == 5);

	arch = dpkg_arch_find("arch-xx");
	pkg = pkg_hash_find_pkg("pkg-aa", arch);
	test_pass(pkg != NULL);
	test_str(pkg->set->name, ==, "pkg-aa");
	test_str(pkg->installed.arch->name, ==, "arch-xx");
	test_str(pkg->available.arch->name, ==, "arch-xx");
	test_pass(pkg_hash_count_set() == 3);
	test_pass(pkg_hash_count_pkg() == 5);

	set = pkg_hash_find_set("pkg-aa");
	test_str(set->name, ==, "pkg-aa");
	pkg = pkg_hash_get_singleton(set);
	test_pass(pkg == NULL);
	test_pass(pkg_hash_count_set() == 3);
	test_pass(pkg_hash_count_pkg() == 5);

	pkg = pkg_hash_find_singleton("pkg-bb");
	test_pass(pkg != NULL);
	test_str(pkg->set->name, ==, "pkg-bb");
	test_pass(pkg_hash_count_set() == 3);
	test_pass(pkg_hash_count_pkg() == 5);

	pkg = pkg_hash_find_singleton("pkg-cc");
	test_pass(pkg != NULL);
	test_str(pkg->set->name, ==, "pkg-cc");
	test_pass(pkg_hash_count_set() == 3);
	test_pass(pkg_hash_count_pkg() == 5);

	iter = pkg_hash_iter_new();
	while ((set = pkg_hash_iter_next_set(iter))) {
		if (strcmp(set->name, "pkg-aa") == 0)
			test_str(set->name, ==, "pkg-aa");
		else if (strcmp(set->name, "pkg-bb") == 0)
			test_str(set->name, ==, "pkg-bb");
		else if (strcmp(set->name, "pkg-cc") == 0)
			test_str(set->name, ==, "pkg-cc");
		else
			test_fail("unknown fsys_namenode");
	}
	pkg_hash_iter_free(iter);

	pkginstance = 0;
	iter = pkg_hash_iter_new();
	while ((pkg = pkg_hash_iter_next_pkg(iter))) {
		pkginstance++;
		if (strcmp(pkg->set->name, "pkg-aa") == 0) {
			struct pkgbin *pkgbin = &pkg->installed;

			test_str(pkg->set->name, ==, "pkg-aa");
			if (strcmp(pkgbin->arch->name, "arch-xx") == 0)
				test_str(pkgbin->arch->name, ==, "arch-xx");
			else if (strcmp(pkgbin->arch->name, "arch-yy") == 0)
				test_str(pkgbin->arch->name, ==, "arch-yy");
			else if (strcmp(pkgbin->arch->name, "arch-zz") == 0)
				test_str(pkgbin->arch->name, ==, "arch-zz");
			else
				test_fail("unknown pkginfo instance");
		} else if (strcmp(pkg->set->name, "pkg-bb") == 0) {
			test_str(pkg->set->name, ==, "pkg-bb");
		} else if (strcmp(pkg->set->name, "pkg-cc") == 0) {
			test_str(pkg->set->name, ==, "pkg-cc");
		} else {
			test_fail("unknown fsys_namenode");
		}
	}
	pkg_hash_iter_free(iter);
	test_pass(pkginstance == 5);

	pkg_hash_reset();
	test_pass(pkg_hash_count_set() == 0);
	test_pass(pkg_hash_count_pkg() == 0);
}

TEST_ENTRY(test)
{
	test_plan(72);

	test_pkg_hash();
}
