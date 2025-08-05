/*
 * libdpkg - Debian packaging suite library routines
 * strhash.c - FNV string hashing support
 *
 * Copyright © 2003 Daniel Silverstone <dsilvers@digital-scurf.org>
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

#include <dpkg/string.h>

#define FNV_OFFSET_BASIS 2166136261UL
#define FNV_MIXING_PRIME 16777619UL

/**
 * Fowler/Noll/Vo -- FNV-1a simple string hash.
 *
 * For more info, @see <http://www.isthe.com/chongo/tech/comp/fnv/index.html>.
 *
 * @param str The string to hash.
 *
 * @return The hashed value.
 */
unsigned int
str_fnv_hash(const char *str)
{
	unsigned int h = FNV_OFFSET_BASIS;
	unsigned int p = FNV_MIXING_PRIME;

	while (*str) {
		h ^= *str++;
		h *= p;
	}

	return h;
}
