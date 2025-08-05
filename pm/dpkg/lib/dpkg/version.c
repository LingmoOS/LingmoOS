/*
 * libdpkg - Debian packaging suite library routines
 * version.c - version handling functions
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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

#include <dpkg/c-ctype.h>
#include <dpkg/ehandle.h>
#include <dpkg/string.h>
#include <dpkg/version.h>

/**
 * Turn the passed version into an empty version.
 *
 * This can be used to ensure the version is properly initialized.
 *
 * @param version The version to clear.
 */
void
dpkg_version_blank(struct dpkg_version *version)
{
	version->epoch = 0;
	version->version = NULL;
	version->revision = NULL;
}

/**
 * Test if a version is not empty.
 *
 * @param version The version to test.
 *
 * @retval true If the version is informative (i.e. not an empty version).
 * @retval false If the version is empty.
 */
bool
dpkg_version_is_informative(const struct dpkg_version *version)
{
	return (version->epoch ||
	        str_is_set(version->version) ||
	        str_is_set(version->revision));
}

/**
 * Give a weight to the character to order in the version comparison.
 *
 * @param c An ASCII character.
 */
static int
order(int c)
{
	if (c_isdigit(c))
		return 0;
	else if (c_isalpha(c))
		return c;
	else if (c == '~')
		return -1;
	else if (c)
		return c + 256;

	return 0;
}

static int
verrevcmp(const char *a, const char *b)
{
	if (a == NULL)
		a = "";
	if (b == NULL)
		b = "";

	while (*a || *b) {
		int first_diff = 0;

		while ((*a && !c_isdigit(*a)) || (*b && !c_isdigit(*b))) {
			int ac = order(*a);
			int bc = order(*b);

			if (ac != bc)
				return ac - bc;

			a++;
			b++;
		}
		while (*a == '0')
			a++;
		while (*b == '0')
			b++;
		while (c_isdigit(*a) && c_isdigit(*b)) {
			if (!first_diff)
				first_diff = *a - *b;
			a++;
			b++;
		}

		if (c_isdigit(*a))
			return 1;
		if (c_isdigit(*b))
			return -1;
		if (first_diff)
			return first_diff;
	}

	return 0;
}

/**
 * Compares two Debian versions.
 *
 * This function follows the convention of the comparator functions used by
 * qsort().
 *
 * @see deb-version(5)
 *
 * @param a The first version.
 * @param b The second version.
 *
 * @retval 0 If a and b are equal.
 * @retval <0 If a is smaller than b.
 * @retval >0 If a is greater than b.
 */
int
dpkg_version_compare(const struct dpkg_version *a,
                     const struct dpkg_version *b)
{
	int rc;

	if (a->epoch > b->epoch)
		return 1;
	if (a->epoch < b->epoch)
		return -1;

	rc = verrevcmp(a->version, b->version);
	if (rc)
		return rc;

	return verrevcmp(a->revision, b->revision);
}

/**
 * Check if two versions have a certain relation.
 *
 * @param a The first version.
 * @param rel The relation.
 * @param b The second version.
 *
 * @retval true If rel is #DPKG_RELATION_NONE or the expression “a rel b” is
 *              true.
 * @retval false Otherwise.
 *
 * @warning If rel is not a valid relation, this function will terminate
 *          the program.
 */
bool
dpkg_version_relate(const struct dpkg_version *a,
                    enum dpkg_relation rel,
                    const struct dpkg_version *b)
{
	int rc;

	if (rel == DPKG_RELATION_NONE)
		return true;

	rc = dpkg_version_compare(a, b);

	switch (rel) {
	case DPKG_RELATION_EQ:
		return rc == 0;
	case DPKG_RELATION_LT:
		return rc < 0;
	case DPKG_RELATION_LE:
		return rc <= 0;
	case DPKG_RELATION_GT:
		return rc > 0;
	case DPKG_RELATION_GE:
		return rc >= 0;
	default:
		internerr("unknown dpkg_relation %d", rel);
	}
	return false;
}
