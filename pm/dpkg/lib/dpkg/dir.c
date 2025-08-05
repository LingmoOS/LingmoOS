/*
 * libdpkg - Debian packaging suite library routines
 * dir.c - directory handling functions
 *
 * Copyright © 2010 Guillem Jover <guillem@debian.org>
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
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <dpkg/dpkg.h>
#include <dpkg/i18n.h>
#include <dpkg/dir.h>

static int
dir_make_path_noalloc(char *dirname, mode_t mode)
{
	char *slash;

	/* Find the first slash, and ignore it, as it will be either the
	 * slash for the root directory, for the current directory in a
	 * relative pathname or its parent. */
	slash = strchr(dirname, '/');

	while (slash != NULL) {
		slash = strchr(slash + 1, '/');
		if (slash)
			*slash = '\0';

		if (mkdir(dirname, mode) < 0 && errno != EEXIST)
			return -1;
		if (slash)
			*slash = '/';
	}

	return 0;
}

/**
 * Create the directory and all its parents if necessary.
 *
 * @param path The pathname to create directories for.
 * @param mode The pathname mode.
 */
int
dir_make_path(const char *path, mode_t mode)
{
	char *dirname;
	int rc;

	dirname = m_strdup(path);
	rc = dir_make_path_noalloc(dirname, mode);
	free(dirname);

	return rc;
}

int
dir_make_path_parent(const char *path, mode_t mode)
{
	char *dirname, *slash;
	int rc;

	dirname = m_strdup(path);
	slash = strrchr(dirname, '/');
	if (slash != NULL) {
		*slash = '\0';
		rc = dir_make_path_noalloc(dirname, mode);
	} else {
		rc = -1;
	}
	free(dirname);

	return rc;
}

/**
 * Sync a directory to disk from a DIR structure.
 *
 * @param dir The directory to sync.
 * @param path The name of the directory to sync (for error messages).
 */
static void
dir_sync(DIR *dir, const char *path)
{
#ifdef HAVE_FSYNC_DIR
	int fd;

	fd = dirfd(dir);
	if (fd < 0)
		ohshite(_("unable to get file descriptor for directory '%s'"),
		        path);

	if (fsync(fd))
		ohshite(_("unable to sync directory '%s'"), path);
#endif
}

/**
 * Sync a directory to disk from a pathname.
 *
 * @param path The name of the directory to sync.
 */
void
dir_sync_path(const char *path)
{
#ifdef HAVE_FSYNC_DIR
	DIR *dir;

	dir = opendir(path);
	if (!dir)
		ohshite(_("unable to open directory '%s'"), path);

	dir_sync(dir, path);

	closedir(dir);
#endif
}

/**
 * Sync to disk the parent directory of a pathname.
 *
 * @param path The child pathname of the directory to sync.
 */
void
dir_sync_path_parent(const char *path)
{
#ifdef HAVE_FSYNC_DIR
	char *dirname, *slash;

	dirname = m_strdup(path);

	slash = strrchr(dirname, '/');
	if (slash != NULL) {
		*slash = '\0';
		dir_sync_path(dirname);
	}

	free(dirname);
#endif
}

/* TODO: Ideally we'd use openat() here, to avoid the path mangling, but
 * it's not widely available yet, so this will do for now. */
static void
dir_file_sync(const char *dir, const char *filename)
{
	char *path;
	int fd;

	path = str_fmt("%s/%s", dir, filename);

	fd = open(path, O_WRONLY);
	if (fd < 0)
		ohshite(_("unable to open file '%s'"), path);
	if (fsync(fd))
		ohshite(_("unable to sync file '%s'"), path);
	if (close(fd))
		ohshite(_("unable to close file '%s'"), path);

	free(path);
}

/**
 * Sync to disk the contents and the directory specified.
 *
 * @param path The pathname to sync.
 */
void
dir_sync_contents(const char *path)
{
	DIR *dir;
	struct dirent *dent;

	dir = opendir(path);
	if (!dir)
		ohshite(_("unable to open directory '%s'"), path);

	while ((dent = readdir(dir)) != NULL) {
		if (strcmp(dent->d_name, ".") == 0 ||
		    strcmp(dent->d_name, "..") == 0)
			continue;

		dir_file_sync(path, dent->d_name);
	}

	dir_sync(dir, path);

	closedir(dir);
}
