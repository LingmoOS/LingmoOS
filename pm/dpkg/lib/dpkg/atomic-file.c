/*
 * libdpkg - Debian packaging suite library routines
 * atomic-file.c - atomic file helper functions
 *
 * Copyright © 2011-2014 Guillem Jover <guillem@debian.org>
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

#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dir.h>
#include <dpkg/atomic-file.h>

#define ATOMIC_FILE_NEW_EXT "-new"
#define ATOMIC_FILE_OLD_EXT "-old"

struct atomic_file *
atomic_file_new(const char *filename, enum atomic_file_flags flags)
{
	struct atomic_file *file;

	file = m_malloc(sizeof(*file));
	file->flags = flags;
	file->fp = NULL;
	file->name = m_strdup(filename);
	file->name_new = str_fmt("%s%s", filename, ATOMIC_FILE_NEW_EXT);

	return file;
}

void
atomic_file_open(struct atomic_file *file)
{
	file->fp = fopen(file->name_new, "w");
	if (file->fp == NULL && file->flags & ATOMIC_FILE_MKPATH) {
		if (dir_make_path_parent(file->name, 0755) < 0)
			ohshite(_("cannot create base directory for %s"),
			        file->name);

		file->fp = fopen(file->name_new, "w");
	}
	if (file->fp == NULL)
		ohshite(_("unable to create new file '%.250s'"),
		        file->name_new);
	fchmod(fileno(file->fp), 0644);

	push_cleanup(cu_closestream, ~ehflag_normaltidy, 1, file->fp);
}

void
atomic_file_sync(struct atomic_file *file)
{
	if (ferror(file->fp))
		ohshite(_("unable to write new file '%.250s'"), file->name_new);
	if (fflush(file->fp))
		ohshite(_("unable to flush new file '%.250s'"), file->name_new);
	if (fsync(fileno(file->fp)))
		ohshite(_("unable to sync new file '%.250s'"), file->name_new);
}

void
atomic_file_close(struct atomic_file *file)
{
	pop_cleanup(ehflag_normaltidy); /* fopen */

	if (fclose(file->fp))
		ohshite(_("unable to close new file '%.250s'"), file->name_new);
}

static void
atomic_file_backup(struct atomic_file *file)
{
	char *name_old;

	name_old = str_fmt("%s%s", file->name, ATOMIC_FILE_OLD_EXT);

	if (unlink(name_old) && errno != ENOENT)
		ohshite(_("error removing old backup file '%s'"), name_old);
	if (link(file->name, name_old) && errno != ENOENT)
		ohshite(_("error creating new backup file '%s'"), name_old);

	free(name_old);
}

void
atomic_file_remove(struct atomic_file *file)
{
	if (unlink(file->name_new))
		ohshite(_("cannot remove '%.250s'"), file->name_new);
	if (unlink(file->name) && errno != ENOENT)
		ohshite(_("cannot remove '%.250s'"), file->name);
}

void
atomic_file_commit(struct atomic_file *file)
{
	if (file->flags & ATOMIC_FILE_BACKUP)
		atomic_file_backup(file);

	if (rename(file->name_new, file->name))
		ohshite(_("error installing new file '%s'"), file->name);
}

void
atomic_file_free(struct atomic_file *file)
{
	free(file->name_new);
	free(file->name);
	free(file);
}
