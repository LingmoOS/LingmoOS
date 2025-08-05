/*
 * libdpkg - Debian packaging suite library routines
 * meminfo.h - system memory information functions
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

#ifndef LIBDPKG_MEMINFO_H
#define LIBDPKG_MEMINFO_H

#include <stdint.h>

#include <dpkg/macros.h>

DPKG_BEGIN_DECLS

/**
 * @defgroup meminfo Memory information handling
 * @ingroup dpkg-internal
 * @{
 */

enum meminfo_error_code {
	MEMINFO_OK = 0,
	MEMINFO_NO_FILE = -1,
	MEMINFO_NO_DATA = -2,
	MEMINFO_INT_NEG = -3,
	MEMINFO_INT_MAX = -4,
	MEMINFO_NO_UNIT = -5,
	MEMINFO_NO_INFO = -6,
};

enum meminfo_error_code
meminfo_get_available_from_file(const char *filename, uint64_t *val);
enum meminfo_error_code
meminfo_get_available(uint64_t *val);

/** @} */

DPKG_END_DECLS

#endif /* LIBDPKG_MEMINFO_H */
