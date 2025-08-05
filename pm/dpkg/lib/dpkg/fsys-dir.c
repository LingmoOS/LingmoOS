/*
 * libdpkg - Debian packaging suite library routines
 * fsys-dir.c - filesystem root directory functions
 *
 * Copyright © 2011, 2018 Guillem Jover <guillem@debian.org>
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
#include <dpkg/string.h>
#include <dpkg/file.h>
#include <dpkg/path.h>
#include <dpkg/fsys.h>

static char *fsys_dir;

static char *
dpkg_fsys_normalize_dir(const char *dir)
{
	char *dir_new;

	dir_new = file_canonicalize(dir);
	path_trim_slash_slashdot(dir_new);

	if (strcmp(dir_new, "/") == 0)
		dir_new[0] = '\0';

	return dir_new;
}

/**
 * Allocate the default on-disk filesystem root directory.
 *
 * The directory defaults to the value from environment variable
 * DPKG_ROOT, and if not set the built-in default "".
 *
 * @return The filesystem root directory.
 */
static char *
dpkg_fsys_new_dir(void)
{
	const char *env;
	char *dir;

	env = getenv("DPKG_ROOT");
	if (env == NULL) {
		dir = m_strdup("");
	} else {
		dir = dpkg_fsys_normalize_dir(env);
	}

	return dir;
}

/**
 * Set current on-disk filesystem root directory.
 *
 * This function can be used to set the directory to a new value, or to
 * reset it to a default value if dir is NULL.
 *
 * @param dir The new filesystem root directory, or NULL to set to default.
 *
 * @return The new filesystem root directory.
 */
const char *
dpkg_fsys_set_dir(const char *dir)
{
	char *dir_new;

	if (dir == NULL) {
		dir_new = dpkg_fsys_new_dir();
	} else {
		dir_new = dpkg_fsys_normalize_dir(dir);
	}

	free(fsys_dir);
	fsys_dir = dir_new;

	return fsys_dir;
}

/**
 * Get current on-disk filesystem root directory.
 *
 * This function will take care of initializing the directory if it has not
 * been initialized before.
 *
 * @return The current filesystem root directory.
 */
const char *
dpkg_fsys_get_dir(void)
{
	if (fsys_dir == NULL)
		fsys_dir = dpkg_fsys_new_dir();

	return fsys_dir;
}

/**
 * Get a pathname to the current on-disk filesystem root directory.
 *
 * This function returns an allocated string, which should be freed with
 * free(2).
 *
 * @param pathpart The pathpart to append to the new pathname.
 *
 * @return The newly allocated pathname.
 */
char *
dpkg_fsys_get_path(const char *pathpart)
{
	pathpart = path_skip_slash_dotslash(pathpart);

	return str_fmt("%s/%s", dpkg_fsys_get_dir(), pathpart);
}
