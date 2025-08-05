/*
 * libdpkg - Debian packaging suite library routines
 * t-pkg-format.c - test pkg-format implementation
 *
 * Copyright © 2022 Guillem Jover <guillem@debian.org>
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
#include <dpkg/pkg.h>
#include <dpkg/pkg-format.h>

static void
prep_pkg(struct pkgset *set, struct pkginfo *pkg)
{
	pkg_blank(pkg);

	pkgset_blank(set);
	pkgset_link_pkg(set, pkg);

	set->name = "test-bin";
	pkg->installed.description = "short synopsis -- some package\n"
	        " This is the extended description for this package-\n"
	        " .\n"
	        " Potentially expanding multiple lines.\n";
	pkg->installed.arch = dpkg_arch_get(DPKG_ARCH_ALL);
	pkg->installed.version = DPKG_VERSION_OBJECT(0, "4.5", "2");
}

static void
prep_virtpkg(struct pkgset *set, struct pkginfo *pkg)
{
	pkg_blank(pkg);

	pkgset_blank(set);
	pkgset_link_pkg(set, pkg);

	set->name = "test-virt";
}

static void
test_field(struct pkginfo *pkg, const char *fmt, const char *exp)
{
	struct pkg_format_node *head;
	struct varbuf vb = VARBUF_INIT;

	head = pkg_format_parse(fmt, NULL);
	test_pass(head);
	pkg_format_print(&vb, head, pkg, &pkg->installed);
	test_str(varbuf_str(&vb), ==, exp);
	pkg_format_free(head);
	varbuf_destroy(&vb);
}

static void
test_pkg_format_real_fields(void)
{
	struct pkgset pkgset;
	struct pkginfo pkg;

	prep_pkg(&pkgset, &pkg);

	test_field(&pkg, "${Package}_${Version}_${Architecture}",
	                 "test-bin_4.5-2_all");
}

static void
test_pkg_format_virtual_fields(void)
{
	struct pkgset pkgset;
	struct pkginfo pkg;

	prep_pkg(&pkgset, &pkg);

	test_field(&pkg, "${source:Package}_${source:Version}",
	                 "test-bin_4.5-2");

	pkg.installed.source = "test-src";
	test_field(&pkg, "${source:Package}_${source:Version}",
	                 "test-src_4.5-2");
	test_field(&pkg, "${source:Upstream-Version}",
	                 "4.5");

	pkg.installed.source = "test-src (1:3.4-6)";
	test_field(&pkg, "${source:Package}_${source:Version}",
	                 "test-src_1:3.4-6");
	test_field(&pkg, "${source:Upstream-Version}",
	                 "3.4");

	test_field(&pkg, "${binary:Synopsis}",
	                 "short synopsis -- some package");

	test_field(&pkg, "${binary:Summary}",
	                 "short synopsis -- some package");

	prep_virtpkg(&pkgset, &pkg);
	test_field(&pkg, "${source:Package}_${source:Version}",
	                 "test-virt_");
	test_field(&pkg, "${source:Upstream-Version}",
	                 "");
}

static void
test_pkg_format_virtual_fields_db_fsys(void)
{
	struct pkg_format_node *head;

	head = pkg_format_parse("prefix ${unknown-variable} suffix", NULL);
	test_pass(head);
	test_fail(pkg_format_needs_db_fsys(head));
	pkg_format_free(head);

	head = pkg_format_parse("prefix ${db-fsys:Files} suffix", NULL);
	test_pass(head);
	test_pass(pkg_format_needs_db_fsys(head));
	pkg_format_free(head);
}

TEST_ENTRY(test)
{
	test_plan(24);

	test_pkg_format_real_fields();
	test_pkg_format_virtual_fields();
	test_pkg_format_virtual_fields_db_fsys();
}
