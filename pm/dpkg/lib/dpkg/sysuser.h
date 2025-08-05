/*
 * libdpkg - Debian packaging suite library routines
 * sysuser.h - system user and group handling
 *
 * Copyright © 2023 Guillem Jover <guillem@debian.org>
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

#ifndef LIBDPKG_SYSUSER_H
#define LIBDPKG_SYSUSER_H

#include <sys/types.h>

#include <pwd.h>
#include <grp.h>

#include <dpkg/dpkg.h>

DPKG_BEGIN_DECLS

struct passwd *
dpkg_sysuser_from_name(const char *uname);
struct passwd *
dpkg_sysuser_from_uid(uid_t uid);

struct group *
dpkg_sysgroup_from_name(const char *gname);
struct group *
dpkg_sysgroup_from_gid(gid_t gid);

DPKG_END_DECLS

#endif /* LIBDPKG_SYSUSER_H */
