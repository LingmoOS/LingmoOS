/*
 * libdpkg - Debian packaging suite library routines
 * t-pkg-queue.c - test pkg-queue implementation
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
#include <dpkg/pkg-queue.h>

static void
test_pkg_queue_init(void)
{
	struct pkg_queue q = PKG_QUEUE_INIT;
	struct pkg_list l;

	test_pass(q.length == 0);
	test_pass(q.head == NULL);
	test_pass(q.tail == NULL);

	test_pass(pkg_queue_is_empty(&q));

	q = (struct pkg_queue){ .length = 10, .head = &l, .tail = &l };

	pkg_queue_init(&q);
	test_pass(q.length == 0);
	test_pass(q.head == NULL);
	test_pass(q.tail == NULL);

	test_pass(pkg_queue_is_empty(&q));
}

static void
test_pkg_queue_push_pop(void)
{
	struct pkg_queue q = PKG_QUEUE_INIT;
	struct pkg_list *l1, *l2, *l3;
	struct pkginfo *pkgp, pkg1, pkg2, pkg3;

	test_pass(pkg_queue_is_empty(&q));

	/* Test push operations. */

	l1 = pkg_queue_push(&q, &pkg1);
	test_pass(l1 != NULL);
	test_pass(q.head == l1);
	test_pass(q.tail == l1);
	test_pass(q.length == 1);

	l2 = pkg_queue_push(&q, &pkg2);
	test_pass(l2 != NULL);
	test_pass(q.head == l1);
	test_pass(q.tail == l2);
	test_pass(q.length == 2);

	l3 = pkg_queue_push(&q, &pkg3);
	test_pass(l3 != NULL);
	test_pass(q.head == l1);
	test_pass(q.tail == l3);
	test_pass(q.length == 3);

	/* Test pop operations. */

	pkgp = pkg_queue_pop(&q);
	test_pass(pkgp == &pkg1);
	test_pass(q.head == l2);
	test_pass(q.tail == l3);
	test_pass(q.length == 2);

	pkgp = pkg_queue_pop(&q);
	test_pass(pkgp == &pkg2);
	test_pass(q.head == l3);
	test_pass(q.tail == l3);
	test_pass(q.length == 1);

	pkgp = pkg_queue_pop(&q);
	test_pass(pkgp == &pkg3);
	test_pass(q.head == NULL);
	test_pass(q.tail == NULL);
	test_pass(q.length == 0);

	test_pass(pkg_queue_is_empty(&q));

	pkgp = pkg_queue_pop(&q);
	test_pass(pkgp == NULL);
	test_pass(q.head == NULL);
	test_pass(q.tail == NULL);
	test_pass(q.length == 0);

	pkg_queue_destroy(&q);
}

TEST_ENTRY(test)
{
	test_plan(38);

	test_pkg_queue_init();
	test_pkg_queue_push_pop();
}
