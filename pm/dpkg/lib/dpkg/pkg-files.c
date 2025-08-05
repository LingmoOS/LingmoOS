/*
 * libdpkg - Debian packaging suite library routines
 * pkg-files.c - handle list of filesystem files per package
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2000,2001 Wichert Akkerman <wakkerma@debian.org>
 * Copyright © 2008-2014 Guillem Jover <guillem@debian.org>
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

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg-list.h>
#include <dpkg/pkg-files.h>

/**
 * Erase the files saved in pkg.
 */
void
pkg_files_blank(struct pkginfo *pkg)
{
	struct fsys_namenode_list *current;

	for (current = pkg->files;
	     current;
	     current = current->next) {
		struct pkg_list **pkg_prev = &current->namenode->packages;
		struct pkg_list *pkg_node;

		/* For each file that used to be in the package, go through
		 * looking for this package's entry in the list of packages
		 * containing this file, and blank it out. */
		for (pkg_node = current->namenode->packages;
		     pkg_node;
		     pkg_node = pkg_node->next) {
			if (pkg_node->pkg == pkg) {
				*pkg_prev = pkg_node->next;

				/* The actual filelist links were allocated
				 * w/ nfmalloc, so we should not free them. */
				break;
			}

			pkg_prev = &pkg_node->next;
		}
	}
	pkg->files = NULL;
}

struct fsys_namenode_list **
pkg_files_add_file(struct pkginfo *pkg, struct fsys_namenode *namenode,
                   struct fsys_namenode_list **file_tail)
{
	struct fsys_namenode_list *newent;
	struct pkg_list *pkg_node;

	if (file_tail == NULL)
		file_tail = &pkg->files;

	/* Make sure we're at the end. */
	while ((*file_tail) != NULL)
		file_tail = &((*file_tail)->next);

	/* Create a new node. */
	newent = nfmalloc(sizeof(*newent));
	newent->namenode = namenode;
	newent->next = NULL;
	*file_tail = newent;
	file_tail = &newent->next;

	/* Add pkg to newent's package list. */
	pkg_node = nfmalloc(sizeof(*pkg_node));
	pkg_node->pkg = pkg;
	pkg_node->next = newent->namenode->packages;
	newent->namenode->packages = pkg_node;

	/* Return the position for the next guy. */
	return file_tail;
}
