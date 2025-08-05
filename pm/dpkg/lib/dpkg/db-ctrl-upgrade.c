/*
 * libdpkg - Debian packaging suite library routines
 * db-ctrl-upgrade.c - package control information database format upgrade
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2011-2014 Guillem Jover <guillem@debian.org>
 * Copyright © 2011 Linaro Limited
 * Copyright © 2011 Raphaël Hertzog <hertzog@debian.org>
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
#include <unistd.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/fsys.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/path.h>
#include <dpkg/dir.h>

struct rename_node {
	struct rename_node *next;
	char *old;
	char *new;
};

/* Global variables. */
static struct rename_node *rename_head = NULL;

static struct rename_node *
rename_node_new(const char *old, const char *new, struct rename_node *next)
{
	struct rename_node *node;

	node = m_malloc(sizeof(*node));
	node->next = next;
	node->old = m_strdup(old);
	node->new = m_strdup(new);

	return node;
}

static void
rename_node_free(struct rename_node *node)
{
	free(node->old);
	free(node->new);
	free(node);
}

static void
pkg_infodb_link_multiarch_files(void)
{
	DIR *db_dir;
	struct dirent *db_de;
	struct varbuf pkgname = VARBUF_INIT;
	struct varbuf oldname = VARBUF_INIT;
	struct varbuf newname = VARBUF_INIT;
	struct varbuf_state oldname_state;
	struct varbuf_state newname_state;

	varbuf_add_dir(&oldname, pkg_infodb_get_dir());
	varbuf_snapshot(&oldname, &oldname_state);

	varbuf_set_varbuf(&newname, &oldname);
	varbuf_snapshot(&newname, &newname_state);

	db_dir = opendir(pkg_infodb_get_dir());
	if (!db_dir)
		ohshite(_("cannot read info directory"));

	push_cleanup(cu_closedir, ~0, 1, (void *)db_dir);
	while ((db_de = readdir(db_dir)) != NULL) {
		const char *filetype, *dot;
		struct pkginfo *pkg;
		struct pkgset *set;

		/* Ignore dotfiles, including ‘.’ and ‘..’. */
		if (db_de->d_name[0] == '.')
			continue;

		/* Ignore anything odd. */
		dot = strrchr(db_de->d_name, '.');
		if (dot == NULL)
			continue;

		varbuf_set_buf(&pkgname, db_de->d_name, dot - db_de->d_name);

		 /* Skip files already converted. */
		if (strchr(pkgname.buf, ':'))
			continue;

		set = pkg_hash_find_set(pkgname.buf);
		for (pkg = &set->pkg; pkg; pkg = pkg->arch_next)
			if (pkg->status != PKG_STAT_NOTINSTALLED)
				break;
		if (!pkg) {
			warning(_("info file %s/%s not associated to any package"),
			        pkg_infodb_get_dir(), db_de->d_name);
			continue;
		}

		/* Does it need to be upgraded? */
		if (pkg->installed.multiarch != PKG_MULTIARCH_SAME)
			continue;

		/* Skip past the full stop. */
		filetype = dot + 1;

		varbuf_rollback(&oldname_state);
		varbuf_add_str(&oldname, db_de->d_name);

		varbuf_rollback(&newname_state);
		varbuf_add_pkgbin_name(&newname, pkg, &pkg->installed, pnaw_always);
		varbuf_add_char(&newname, '.');
		varbuf_add_str(&newname, filetype);

		if (link(oldname.buf, newname.buf) && errno != EEXIST)
			ohshite(_("error creating hard link '%.255s'"),
			        newname.buf);
		rename_head = rename_node_new(oldname.buf, newname.buf, rename_head);
	}
	pop_cleanup(ehflag_normaltidy); /* closedir */

	varbuf_destroy(&pkgname);
	varbuf_destroy(&newname);
	varbuf_destroy(&oldname);
}

static void
cu_abort_db_upgrade(int argc, void **argv)
{
	struct atomic_file *file = argv[0];
	struct rename_node *next;

	/* Restore the old files if needed and drop the newly created files. */
	while (rename_head) {
		next = rename_head->next;
		if (link(rename_head->new, rename_head->old) && errno != EEXIST)
			ohshite(_("error creating hard link '%.255s'"),
			        rename_head->old);
		if (unlink(rename_head->new))
			ohshite(_("cannot remove '%.250s'"), rename_head->new);
		rename_node_free(rename_head);
		rename_head = next;
	}
	if (unlink(file->name_new) && errno != ENOENT)
		ohshite(_("cannot remove '%.250s'"), file->name_new);

	atomic_file_free(file);
}

static void
pkg_infodb_write_format(struct atomic_file *file, int version)
{
	if (fprintf(file->fp, "%d\n", version) < 0)
		ohshite(_("error while writing '%s'"), file->name_new);

	atomic_file_sync(file);
	atomic_file_close(file);
	dir_sync_path_parent(file->name);

	pkg_infodb_set_format(version);
}

static void
pkg_infodb_unlink_monoarch_files(void)
{
	struct rename_node *next;

	while (rename_head) {
		next = rename_head->next;
		if (unlink(rename_head->old))
			ohshite(_("cannot remove '%.250s'"), rename_head->old);
		rename_node_free(rename_head);
		rename_head = next;
	}
}

static void
pkg_infodb_upgrade_to_multiarch(void)
{
	struct atomic_file *db_file;
	char *db_format_file;

	db_format_file = dpkg_db_get_path(INFODIR "/format");
	db_file = atomic_file_new(db_format_file, ATOMIC_FILE_MKPATH);
	atomic_file_open(db_file);

	push_cleanup(cu_abort_db_upgrade, ehflag_bombout, 1, db_file);

	pkg_infodb_link_multiarch_files();
	pkg_infodb_write_format(db_file, 1);

	pkg_infodb_unlink_monoarch_files();
	atomic_file_commit(db_file);
	dir_sync_path(pkg_infodb_get_dir());

	pop_cleanup(ehflag_normaltidy);

	atomic_file_free(db_file);
	free(db_format_file);
}

/**
 * Upgrade the infodb if there's the need and possibility.
 *
 * Currently this implies, that the modstatdb was opened for writing and:
 * - previous upgrade has not been completed; or
 * - current format is not the latest one.
 */
void
pkg_infodb_upgrade(void)
{
	enum pkg_infodb_format db_format;

	/* Make sure to always read and verify the format version. */
	db_format = pkg_infodb_get_format();

	if (modstatdb_get_status() < msdbrw_write)
		return;

	if (db_format < PKG_INFODB_FORMAT_MULTIARCH ||
	    pkg_infodb_is_upgrading())
		pkg_infodb_upgrade_to_multiarch();
}
