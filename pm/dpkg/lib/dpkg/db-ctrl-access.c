/*
 * libdpkg - Debian packaging suite library routines
 * db-ctrl-access.c - package control information database
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <dirent.h>
#include <unistd.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/fsys.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/debug.h>

bool
pkg_infodb_has_file(struct pkginfo *pkg, struct pkgbin *pkgbin,
                    const char *name)
{
	const char *filename;
	struct stat stab;

	filename = pkg_infodb_get_file(pkg, pkgbin, name);
	if (lstat(filename, &stab) == 0)
		return true;
	else if (errno == ENOENT)
		return false;
	else
		ohshite(_("unable to check existence of '%.250s'"), filename);
}

void
pkg_infodb_foreach(struct pkginfo *pkg, struct pkgbin *pkgbin,
                   pkg_infodb_file_func *func)
{
	DIR *db_dir;
	struct dirent *db_de;
	struct varbuf_state db_path_state;
	struct varbuf db_path = VARBUF_INIT;
	const char *pkgname;
	enum pkg_infodb_format db_format;

	/* Make sure to always read and verify the format version. */
	db_format = pkg_infodb_get_format();

	if (pkgbin->multiarch == PKG_MULTIARCH_SAME &&
	    db_format == PKG_INFODB_FORMAT_MULTIARCH)
		pkgname = pkgbin_name(pkg, pkgbin, pnaw_always);
	else
		pkgname = pkgbin_name(pkg, pkgbin, pnaw_never);

	varbuf_add_dir(&db_path, pkg_infodb_get_dir());
	varbuf_snapshot(&db_path, &db_path_state);

	db_dir = opendir(db_path.buf);
	if (!db_dir)
		ohshite(_("cannot read info directory"));

	push_cleanup(cu_closedir, ~0, 1, (void *)db_dir);
	while ((db_de = readdir(db_dir)) != NULL) {
		const char *filename, *filetype, *dot;

		debug(dbg_veryverbose, "infodb foreach info file '%s'",
		      db_de->d_name);

		/* Ignore dotfiles, including ‘.’ and ‘..’. */
		if (db_de->d_name[0] == '.')
			continue;

		/* Ignore anything odd. */
		dot = strrchr(db_de->d_name, '.');
		if (dot == NULL)
			continue;

		/* Ignore files from other packages. */
		if (strlen(pkgname) != (size_t)(dot - db_de->d_name) ||
		    strncmp(db_de->d_name, pkgname, dot - db_de->d_name))
			continue;

		debug(dbg_stupidlyverbose, "infodb foreach file this pkg");

		/* Skip past the full stop. */
		filetype = dot + 1;

		varbuf_rollback(&db_path_state);
		varbuf_add_str(&db_path, db_de->d_name);
		filename = db_path.buf;

		func(filename, filetype);
	}
	pop_cleanup(ehflag_normaltidy); /* closedir */

	varbuf_destroy(&db_path);
}
