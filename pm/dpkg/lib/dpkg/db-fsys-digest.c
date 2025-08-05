/*
 * libdpkg - Debian packaging suite library routines
 * db-fsys-digest.c - management of filesystem digests database
 *
 * Copyright © 2012-2014 Guillem Jover <guillem@debian.org>
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
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/debug.h>
#include <dpkg/fdio.h>
#include <dpkg/dir.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>

/*
 * If mask is nonzero, will not write any file whose fsys_namenode
 * has any flag bits set in mask.
 */
void
write_filehash_except(struct pkginfo *pkg, struct pkgbin *pkgbin,
                      struct fsys_namenode_list *list, enum fsys_namenode_flags mask)
{
	struct atomic_file *file;
	const char *hashfile;

	debug(dbg_general, "generating infodb hashfile");

	if (pkg_infodb_has_file(pkg, &pkg->available, HASHFILE))
		return;

	hashfile = pkg_infodb_get_file(pkg, pkgbin, HASHFILE);

	file = atomic_file_new(hashfile, 0);
	atomic_file_open(file);

	for (; list; list = list->next) {
		const struct fsys_namenode *namenode = list->namenode;

		if (mask && (namenode->flags & mask))
			continue;
		if (namenode->newhash == NULL)
			continue;

		fprintf(file->fp, "%s  %s\n",
		        namenode->newhash, namenode->name + 1);
	}

	atomic_file_sync(file);
	atomic_file_close(file);
	atomic_file_commit(file);
	atomic_file_free(file);

	dir_sync_path(pkg_infodb_get_dir());
}

static void
parse_filehash_buffer(struct varbuf *buf,
                      struct pkginfo *pkg, struct pkgbin *pkgbin)
{
	char *thisline, *nextline;
	const char *pkgname = pkg_name(pkg, pnaw_nonambig);
	const char *buf_end = buf->buf + buf->used;

	for (thisline = buf->buf; thisline < buf_end; thisline = nextline) {
		struct fsys_namenode *namenode;
		char *endline, *hash_end, *filename;

		endline = memchr(thisline, '\n', buf_end - thisline);
		if (endline == NULL)
			ohshit(_("control file '%s' for package '%s' is "
			         "missing final newline"), HASHFILE, pkgname);

		/* The md5sum hash has a constant length. */
		hash_end = thisline + MD5HASHLEN;

		filename = hash_end + 2;
		if (filename + 1 > endline)
			ohshit(_("control file '%s' for package '%s' is "
			         "missing value"), HASHFILE, pkgname);

		if (hash_end[0] != ' ' || hash_end[1] != ' ')
			ohshit(_("control file '%s' for package '%s' is "
			         "missing value separator"), HASHFILE, pkgname);
		hash_end[0] = '\0';

		/* Where to start next time around. */
		nextline = endline + 1;

		/* Strip trailing ‘/’. */
		if (endline > thisline && endline[-1] == '/')
			endline--;
		*endline = '\0';

		if (endline == thisline)
			ohshit(_("control file '%s' for package '%s' "
			         "contains empty filename"), HASHFILE, pkgname);

		debug(dbg_eachfiledetail, "load digest '%s' for filename '%s'",
		      thisline, filename);

		/* Add the file to the list. */
		namenode = fsys_hash_find_node(filename, FHFF_NONE);
		namenode->newhash = nfstrsave(thisline);
	}
}

void
parse_filehash(struct pkginfo *pkg, struct pkgbin *pkgbin)
{
	const char *hashfile;
	struct varbuf buf = VARBUF_INIT;
	struct dpkg_error err = DPKG_ERROR_INIT;

	hashfile = pkg_infodb_get_file(pkg, pkgbin, HASHFILE);

	if (file_slurp(hashfile, &buf, &err) < 0 && err.syserrno != ENOENT)
		dpkg_error_print(&err,
		                 _("loading control file '%s' for package '%s'"),
		                 HASHFILE, pkg_name(pkg, pnaw_nonambig));

	if (buf.used > 0)
		parse_filehash_buffer(&buf, pkg, pkgbin);

	varbuf_destroy(&buf);
}
