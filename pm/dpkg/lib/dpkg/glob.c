/*
 * libdpkg - Debian packaging suite library routines
 * glob.c - file globing functions
 *
 * Copyright © 2009, 2010 Guillem Jover <guillem@debian.org>
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
#include <dpkg/glob.h>

void
glob_list_prepend(struct glob_node **list, char *pattern)
{
	struct glob_node *node;

	node = m_malloc(sizeof(*node));
	node->pattern = pattern;
	node->next = *list;
	*list = node;
}

void
glob_list_free(struct glob_node *head)
{
	while (head) {
		struct glob_node *node = head;

		head = head->next;
		free(node->pattern);
		free(node);
	}
}
