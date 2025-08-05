/*
 * dpkg - main program for package management
 * perpkgstate.c - struct perpackagestate and function handling
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2000,2001 Wichert Akkerman <wakkerma@debian.org>
 * Copyright © 2008-2014 Guillem Jover <guillem@debian.org>
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

#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>

#include "main.h"

void
ensure_package_clientdata(struct pkginfo *pkg)
{
	if (pkg->clientdata)
		return;
	pkg->clientdata = nfmalloc(sizeof(*pkg->clientdata));
	pkg->clientdata->istobe = PKG_ISTOBE_NORMAL;
	pkg->clientdata->color = PKG_CYCLE_WHITE;
	pkg->clientdata->enqueued = false;
	pkg->clientdata->replacingfilesandsaid = 0;
	pkg->clientdata->cmdline_seen = 0;
	pkg->clientdata->trigprocdeferred = NULL;
}
