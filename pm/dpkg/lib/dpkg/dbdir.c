/*
 * libdpkg - Debian packaging suite library routines
 * dbdir.c - on-disk database directory functions
 *
 * Copyright © 2011 Guillem Jover <guillem@debian.org>
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

#include <sys/types.h>

#include <stdlib.h>

#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/file.h>
#include <dpkg/fsys.h>

static char *db_dir;

/**
 * Allocate the default on-disk database directory.
 *
 * The directory defaults to the value from environment variable
 * DPKG_ADMINDIR, and if not set the built-in default ADMINDIR.
 *
 * @return The database directory.
 */
static char *
dpkg_db_new_dir(void)
{
	const char *env;
	char *dir;

	/* Make sure the filesystem root directory is initialized. */
	dpkg_fsys_get_dir();

	env = getenv("DPKG_ADMINDIR");
	if (env)
		dir = file_canonicalize(env);
	else
		dir = dpkg_fsys_get_path(ADMINDIR);

	return dir;
}

/**
 * Set current on-disk database directory.
 *
 * This function can be used to set the directory to a new value, or to
 * reset it to a default value if dir is NULL.
 *
 * @param dir The new database directory, or NULL to set to default.
 *
 * @return The new database directory.
 */
const char *
dpkg_db_set_dir(const char *dir)
{
	char *dir_new;

	if (dir == NULL)
		dir_new = dpkg_db_new_dir();
	else
		dir_new = file_canonicalize(dir);

	free(db_dir);
	db_dir = dir_new;

	return db_dir;
}

/**
 * Get current on-disk database directory.
 *
 * This function will take care of initializing the directory if it has not
 * been initialized before.
 *
 * @return The current database directory.
 */
const char *
dpkg_db_get_dir(void)
{
	if (db_dir == NULL)
		db_dir = dpkg_db_new_dir();

	return db_dir;
}

/**
 * Get a pathname to the current on-disk database directory.
 *
 * This function returns an allocated string, which should be freed with
 * free(2).
 *
 * @param pathpart The pathpart to append to the new pathname.
 *
 * @return The newly allocated pathname.
 */
char *
dpkg_db_get_path(const char *pathpart)
{
	return str_fmt("%s/%s", dpkg_db_get_dir(), pathpart);
}
