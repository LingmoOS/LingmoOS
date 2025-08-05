/*
 * libdpkg - Debian packaging suite library routines
 * options-dirs.c - CLI options parsing directories
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

#include <config.h>
#include <compat.h>

#include <stdlib.h>

#include <dpkg/macros.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/fsys.h>
#include <dpkg/options.h>

void
set_instdir(const struct cmdinfo *cip, const char *value)
{
	/* Make sure the database is initialized before the root directory,
	 * otherwise, on first use it would get initialized based on the
	 * newly set root directory. */
	dpkg_db_get_dir();

	dpkg_fsys_set_dir(value);
}

void
set_admindir(const struct cmdinfo *cip, const char *value)
{
	dpkg_db_set_dir(value);
}

void
set_root(const struct cmdinfo *cip, const char *value)
{
	char *db_dir;

	/* Initialize the root directory. */
	dpkg_fsys_set_dir(value);

	/* Set the database directory based on the new root directory. */
	db_dir = dpkg_fsys_get_path(ADMINDIR);
	dpkg_db_set_dir(db_dir);
	free(db_dir);
}
