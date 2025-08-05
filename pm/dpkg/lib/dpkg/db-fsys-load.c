/*
 * libdpkg - Debian packaging suite library routines
 * db-fsys-load.c - (re)loading of database of files installed on system
 *
 * Copyright © 2008-2024 Guillem Jover <guillem@debian.org>
 * Copyright © 2022 Simon Richter <sjr@debian.org>
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

#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/db-fsys.h>
#include <dpkg/i18n.h>
#include <dpkg/debug.h>

enum dpkg_db_error
dpkg_db_reopen(struct dpkg_db *db)
{
	struct stat st_next;
	FILE *file_next;

	if (db->pathname == NULL)
		db->pathname = dpkg_db_get_path(db->name);

	onerr_abort++;

	file_next = fopen(db->pathname, "r");
	if (!file_next) {
		if (errno != ENOENT)
			ohshite(_("cannot open %s file"), db->pathname);
	} else {
		setcloexec(fileno(file_next), db->pathname);

		if (fstat(fileno(file_next), &st_next))
			ohshite(_("cannot get %s file metadata"), db->pathname);

		/*
		 * We need to keep the database file open so that the
		 * filesystem cannot reuse the inode number (f.ex. during
		 * multiple dpkg-divert invocations in a maintainer script),
		 * otherwise the following check might turn true, and we
		 * would skip reloading a modified database.
		 */
		if (db->file &&
		    db->st.st_dev == st_next.st_dev &&
		    db->st.st_ino == st_next.st_ino) {
			fclose(file_next);
			onerr_abort--;

			debug(dbg_general, "%s: unchanged %s, skipping",
			      __func__, db->pathname);
			return DPKG_DB_SAME;
		}
		db->st = st_next;
	}
	if (db->file)
		fclose(db->file);
	db->file = file_next;

	onerr_abort--;

	if (db->file) {
		debug(dbg_general, "%s: new %s, (re)loading",
		      __func__, db->pathname);
		return DPKG_DB_LOAD;
	} else {
		debug(dbg_general, "%s: missing %s, resetting",
		      __func__, db->pathname);
		return DPKG_DB_NONE;
	}
}
