/*
 * libdpkg - Debian packaging suite library routines
 * db-fsys-divert.c - management of filesystem diverted files database
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2000, 2001 Wichert Akkerman <wakkerma@debian.org>
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

#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/debug.h>
#include <dpkg/db-fsys.h>

static struct fsys_diversion *diversions = NULL;

void
ensure_diversions(void)
{
	static struct dpkg_db db = {
		.name = DIVERSIONSFILE,
	};
	enum dpkg_db_error rc;
	char linebuf[MAXDIVERTFILENAME];
	struct fsys_diversion *ov, *oicontest, *oialtname;

	rc = dpkg_db_reopen(&db);
	if (rc == DPKG_DB_SAME)
		return;

	for (ov = diversions; ov; ov = ov->next) {
		ov->useinstead->divert->camefrom->divert = NULL;
		ov->useinstead->divert = NULL;
	}
	diversions = NULL;

	if (rc == DPKG_DB_NONE)
		return;

	onerr_abort++;

	while (fgets_checked(linebuf, sizeof(linebuf), db.file, db.pathname) >= 0) {
		oicontest = nfmalloc(sizeof(*oicontest));
		oialtname = nfmalloc(sizeof(*oialtname));

		oialtname->camefrom = fsys_hash_find_node(linebuf, FHFF_NONE);
		oialtname->useinstead = NULL;

		fgets_must(linebuf, sizeof(linebuf), db.file, db.pathname);
		oicontest->useinstead = fsys_hash_find_node(linebuf, FHFF_NONE);
		oicontest->camefrom = NULL;

		fgets_must(linebuf, sizeof(linebuf), db.file, db.pathname);
		oicontest->pkgset = strcmp(linebuf, ":") ?
		                    pkg_hash_find_set(linebuf) : NULL;
		oialtname->pkgset = oicontest->pkgset;

		if (oialtname->camefrom->divert ||
		    oicontest->useinstead->divert)
			ohshit(_("conflicting diversions involving '%.250s' or '%.250s'"),
			       oialtname->camefrom->name, oicontest->useinstead->name);

		oialtname->camefrom->divert = oicontest;
		oicontest->useinstead->divert = oialtname;

		oicontest->next = diversions;
		diversions = oicontest;
	}

	onerr_abort--;
}
