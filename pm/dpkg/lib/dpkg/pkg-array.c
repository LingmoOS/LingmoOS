/*
 * libdpkg - Debian packaging suite library routines
 * pkg-array.c - primitives for pkg array handling
 *
 * Copyright © 1995,1996 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2009-2015 Guillem Jover <guillem@debian.org>
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
#include <stdlib.h>

#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg-spec.h>
#include <dpkg/pkg-array.h>

/**
 * Initialize a package array from package names.
 *
 * @param a          The array to initialize.
 * @param pkg_mapper A function that maps a package name to a package instance.
 * @param pkg_names  The package names list.
 */
void
pkg_array_init_from_names(struct pkg_array *a, pkg_mapper_func pkg_mapper,
                          const char **pkg_names)
{
	int i = 0;

	while (pkg_names[i])
		i++;

	a->n_pkgs = i;
	a->pkgs = m_malloc(sizeof(a->pkgs[0]) * a->n_pkgs);

	for (i = 0; pkg_names[i]; i++)
		a->pkgs[i] = pkg_mapper(pkg_names[i]);
}

/**
 * Initialize a package array from the package database.
 *
 * @param a The array to initialize.
 */
void
pkg_array_init_from_hash(struct pkg_array *a)
{
	struct pkg_hash_iter *iter;
	struct pkginfo *pkg;
	int i;

	a->n_pkgs = pkg_hash_count_pkg();
	a->pkgs = m_malloc(sizeof(a->pkgs[0]) * a->n_pkgs);

	iter = pkg_hash_iter_new();
	for (i = 0; (pkg = pkg_hash_iter_next_pkg(iter)); i++)
		a->pkgs[i] = pkg;
	pkg_hash_iter_free(iter);

	if (i != a->n_pkgs)
		internerr("inconsistent state in pkg array: i=%d != npkgs=%d",
		          i, a->n_pkgs);
}

/**
 * Visit each non-NULL package in a package array.
 *
 * @param a The array to visit.
 * @param pkg_visitor The function to visit each item of the array.
 * @param pkg_data Data to pass pkg_visit for each package visited.
 */
void
pkg_array_foreach(struct pkg_array *a, pkg_array_visitor_func *pkg_visitor,
                  void *pkg_data)
{
	int i;

	for (i = 0; i < a->n_pkgs; i++) {
		struct pkginfo *pkg = a->pkgs[i];

		if (pkg == NULL)
			continue;

		pkg_visitor(a, pkg, pkg_data);
	}
}

/**
 * Sort a package array.
 *
 * @param a The array to sort.
 * @param pkg_sort The function to sort the array.
 */
void
pkg_array_sort(struct pkg_array *a, pkg_sorter_func *pkg_sort)
{
	qsort(a->pkgs, a->n_pkgs, sizeof(a->pkgs[0]), pkg_sort);
}

/**
 * Destroy a package array.
 *
 * Frees the allocated memory and resets the members.
 *
 * @param a The array to destroy.
 */
void
pkg_array_destroy(struct pkg_array *a)
{
	a->n_pkgs = 0;
	free(a->pkgs);
	a->pkgs = NULL;
}
