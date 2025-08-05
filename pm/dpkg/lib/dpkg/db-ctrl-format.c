/*
 * libdpkg - Debian packaging suite library routines
 * db-ctrl-format.c - package control information database format
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

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/varbuf.h>
#include <dpkg/db-ctrl.h>

static enum pkg_infodb_format db_format = PKG_INFODB_FORMAT_UNKNOWN;
static bool db_upgrading;
static char *db_infodir;

static enum pkg_infodb_format
pkg_infodb_parse_format(const char *file)
{
	FILE *fp;
	unsigned int format;

	fp = fopen(file, "r");
	if (fp == NULL) {
		/* A missing format file means legacy format (0). */
		if (errno == ENOENT)
			return PKG_INFODB_FORMAT_LEGACY;
		ohshite(_("error trying to open %.250s"), file);
	}

	if (fscanf(fp, "%u", &format) != 1)
		ohshit(_("corrupt info database format file '%s'"), file);

	fclose(fp);

	return format;
}

static enum pkg_infodb_format
pkg_infodb_read_format(void)
{
	struct atomic_file *file;
	struct stat st;
	char *filename;

	filename = dpkg_db_get_path(INFODIR "/format");
	file = atomic_file_new(filename, 0);

	db_format = pkg_infodb_parse_format(file->name);

	/* Check if a previous upgrade got interrupted. Because we are only
	 * supposed to upgrade the db layout one format at a time, if the
	 * new file exists that means the new format is just one ahead,
	 * we don't try to read it because it contains unreliable data. */
	if (stat(file->name_new, &st) == 0) {
		db_format++;
		db_upgrading = true;
	}

	atomic_file_free(file);
	free(filename);

	if (db_format < 0 || db_format >= PKG_INFODB_FORMAT_LAST)
		ohshit(_("info database format (%d) is bogus or too new; "
		         "try getting a newer dpkg"), db_format);

	return db_format;
}

enum pkg_infodb_format
pkg_infodb_get_format(void)
{
	if (db_format > PKG_INFODB_FORMAT_UNKNOWN)
		return db_format;
	else
		return pkg_infodb_read_format();
}

void
pkg_infodb_set_format(enum pkg_infodb_format version)
{
	db_format = version;
}

bool
pkg_infodb_is_upgrading(void)
{
	if (db_format < 0)
		pkg_infodb_read_format();

	return db_upgrading;
}

const char *
pkg_infodb_get_dir(void)
{
	if (db_infodir == NULL)
		db_infodir = dpkg_db_get_path(INFODIR);

	return db_infodir;
}

const char *
pkg_infodb_get_file(const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                    const char *filetype)
{
	static struct varbuf vb;
	enum pkg_infodb_format format;

	/* Make sure to always read and verify the format version. */
	format = pkg_infodb_get_format();

	varbuf_reset(&vb);
	varbuf_add_dir(&vb, pkg_infodb_get_dir());
	varbuf_add_str(&vb, pkg->set->name);
	if (pkgbin->multiarch == PKG_MULTIARCH_SAME &&
	    format == PKG_INFODB_FORMAT_MULTIARCH)
		varbuf_add_archqual(&vb, pkgbin->arch);
	varbuf_add_char(&vb, '.');
	varbuf_add_str(&vb, filetype);

	return vb.buf;
}

const char *
pkg_infodb_reset_dir(void)
{
	free(db_infodir);
	db_infodir = NULL;

	return pkg_infodb_get_dir();
}
