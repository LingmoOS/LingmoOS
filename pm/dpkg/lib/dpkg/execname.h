/*
 * libdpkg - Debian packaging suite library routines
 * execname.h - executable name handling functions
 *
 * Copyright © 2024 Guillem Jover <guillem@debian.org>
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

#ifndef LIBDPKG_EXECNAME_H
#define LIBDPKG_EXECNAME_H

#include <sys/types.h>

#include <dpkg/macros.h>

DPKG_BEGIN_DECLS

/**
 * @defgroup execname Executable name handling
 * @ingroup dpkg-public
 * @{
 */

char *
dpkg_get_pid_execname(pid_t pid);

/** @} */

DPKG_END_DECLS

#endif
