/*
 * libdpkg - Debian packaging suite library routines
 * t-pkg-list.c - test pkg-list implementation
 *
 * Copyright © 2010,2012 Guillem Jover <guillem@debian.org>
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
#include <dpkg/pkg-list.h>

static void
test_pkg_list_new(void)
{
	struct pkg_list *l1, *l2, *l3;
	struct pkginfo pkg1, pkg2, pkg3;

	l1 = pkg_list_new(&pkg1, NULL);
	test_alloc(l1);
	test_pass(l1->next == NULL);
	test_pass(l1->pkg == &pkg1);

	l2 = pkg_list_new(&pkg2, l1);
	test_alloc(l2);
	test_pass(l2->next == l1);
	test_pass(l2->pkg == &pkg2);

	l3 = pkg_list_new(&pkg3, l2);
	test_alloc(l3);
	test_pass(l3->next == l2);
	test_pass(l3->pkg == &pkg3);

	pkg_list_free(l3);
}

static void
test_pkg_list_prepend(void)
{
	struct pkg_list *head = NULL, *l1, *l2, *l3;
	struct pkginfo pkg1, pkg2, pkg3, pkg4;

	pkg_list_prepend(&head, &pkg1);
	test_alloc(head);
	test_pass(head->next == NULL);
	test_pass(head->pkg == &pkg1);
	l1 = head;

	pkg_list_prepend(&head, &pkg2);
	test_alloc(head);
	test_pass(head->next == l1);
	test_pass(head->pkg == &pkg2);
	l2 = head;

	pkg_list_prepend(&head, &pkg3);
	test_alloc(head);
	test_pass(head->next == l2);
	test_pass(head->pkg == &pkg3);
	l3 = head;

	pkg_list_prepend(&head, &pkg4);
	test_alloc(head);
	test_pass(head->next == l3);
	test_pass(head->pkg == &pkg4);

	pkg_list_free(head);
}

TEST_ENTRY(test)
{
	test_plan(14);

	test_pkg_list_new();
	test_pkg_list_prepend();
}
