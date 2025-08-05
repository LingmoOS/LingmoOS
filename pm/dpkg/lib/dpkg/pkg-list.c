/*
 * libdpkg - Debian packaging suite library routines
 * pkg-list.c - primitives for pkg linked list handling
 *
 * Copyright © 2009 Guillem Jover <guillem@debian.org>
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

#include <stdlib.h>

#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg-list.h>

/**
 * Create a new package list node.
 *
 * @param pkg The pkginfo to assign to the node.
 * @param next The next package list node.
 *
 * @return The new package list node.
 */
struct pkg_list *
pkg_list_new(struct pkginfo *pkg, struct pkg_list *next)
{
	struct pkg_list *node;

	node = m_malloc(sizeof(*node));
	node->pkg = pkg;
	node->next = next;

	return node;
}

/**
 * Free all nodes of a package list.
 *
 * @param head The head of the list to free.
 */
void
pkg_list_free(struct pkg_list *head)
{
	while (head) {
		struct pkg_list *node;

		node = head;
		head = head->next;

		free(node);
	}
}

/**
 * Prepend a package list node to a package list.
 *
 * @param head The head of the list to prepend to.
 * @param pkg The pkginfo to prepend to the list.
 */
void
pkg_list_prepend(struct pkg_list **head, struct pkginfo *pkg)
{
	*head = pkg_list_new(pkg, *head);
}
