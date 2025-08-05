/*
 * dpkg-statoverride - override ownership and mode of files
 *
 * Copyright © 2000, 2001 Wichert Akkerman <wakkerma@debian.org>
 * Copyright © 2006-2015 Guillem Jover <guillem@debian.org>
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
 *
 */

#include <config.h>
#include <compat.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <string.h>
#include <grp.h>
#include <pwd.h>
#include <fnmatch.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/debug.h>
#include <dpkg/string.h>
#include <dpkg/path.h>
#include <dpkg/dir.h>
#include <dpkg/glob.h>
#include <dpkg/sysuser.h>
#include <dpkg/db-fsys.h>
#include <dpkg/options.h>

#include "force.h"
#include "actions.h"
#include "security-mac.h"

static const char printforhelp[] = N_(
"Use --help for help about overriding file stat information.");

static int
printversion(const char *const *argv)
{
	printf(_("Debian %s version %s.\n"), dpkg_get_progname(),
	       PACKAGE_RELEASE);

	printf(_(
"This is free software; see the GNU General Public License version 2 or\n"
"later for copying conditions. There is NO warranty.\n"));

	m_output(stdout, _("<standard output>"));

	return 0;
}

static int
usage(const char *const *argv)
{
	printf(_(
"Usage: %s [<option> ...] <command>\n"
"\n"), dpkg_get_progname());

	printf(_(
"Commands:\n"
"  --add <owner> <group> <mode> <path>\n"
"                           add a new <path> entry into the database.\n"
"  --remove <path>          remove <path> from the database.\n"
"  --list [<glob-pattern>]  list current overrides in the database.\n"
"\n"));

	printf(_(
"Options:\n"
"  --admindir <directory>   set the directory with the statoverride file.\n"
"  --instdir <directory>    set the root directory, but not the admin dir.\n"
"  --root <directory>       set the directory of the root filesystem.\n"
"  --update                 immediately update <path> permissions.\n"
"  --force                  deprecated alias for --force-all.\n"
"  --force-<thing>[,...]    override problems (see --force-help).\n"
"  --no-force-<thing>[,...] stop when problems encountered.\n"
"  --refuse-<thing>[,...]   ditto.\n"
"  --quiet                  quiet operation, minimal output.\n"
"  --help                   show this help message.\n"
"  --version                show the version.\n"
"\n"));

	m_output(stdout, _("<standard output>"));

	return 0;
}

#define FORCE_STATCMD_MASK \
	FORCE_NON_ROOT | \
	FORCE_SECURITY_MAC | FORCE_STATOVERRIDE_ADD | FORCE_STATOVERRIDE_DEL

static int opt_verbose = 1;
static int opt_update = 0;

static char *
path_cleanup(const char *path)
{
	char *new_path = m_strdup(path);

	path_trim_slash_slashdot(new_path);
	if (opt_verbose && strcmp(path, new_path) != 0)
		warning(_("stripping trailing /"));

	return new_path;
}

static struct file_stat *
statdb_node_new(const char *user, const char *group, const char *mode)
{
	struct file_stat *filestat;

	filestat = nfmalloc(sizeof(*filestat));

	filestat->uid = statdb_parse_uid(user);
	if (filestat->uid == (uid_t)-1)
		ohshit(_("user '%s' does not exist"), user);
	filestat->uname = NULL;
	filestat->gid = statdb_parse_gid(group);
	if (filestat->gid == (gid_t)-1)
		ohshit(_("group '%s' does not exist"), group);
	filestat->gname = NULL;
	filestat->mode = statdb_parse_mode(mode);

	return filestat;
}

static struct file_stat **
statdb_node_find(const char *filename)
{
	struct fsys_namenode *file;

	file = fsys_hash_find_node(filename, FHFF_NONE);

	return &file->statoverride;
}

static int
statdb_node_remove(const char *filename)
{
	struct fsys_namenode *file;

	file = fsys_hash_find_node(filename, FHFF_NO_NEW);
	if (!file || !file->statoverride)
		return 0;

	file->statoverride = NULL;

	return 1;
}

static void
statdb_node_apply(const char *filename, struct file_stat *filestat)
{
	int rc;

	rc = chown(filename, filestat->uid, filestat->gid);
	if (forcible_nonroot_error(rc) < 0)
		ohshite(_("error setting ownership of '%.255s'"), filename);
	rc = chmod(filename, filestat->mode & ~S_IFMT);
	if (forcible_nonroot_error(rc) < 0)
		ohshite(_("error setting permissions of '%.255s'"), filename);

	dpkg_selabel_load();
	dpkg_selabel_set_context(filename, filename, filestat->mode);
	dpkg_selabel_close();
}

static void
statdb_node_print(FILE *out, struct fsys_namenode *file)
{
	struct file_stat *filestat = file->statoverride;
	struct passwd *pw;
	struct group *gr;

	if (!filestat)
		return;

	pw = dpkg_sysuser_from_uid(filestat->uid);
	if (pw)
		fprintf(out, "%s ", pw->pw_name);
	else if (filestat->uname)
		fprintf(out, "%s ", filestat->uname);
	else
		fprintf(out, "#%d ", filestat->uid);

	gr = dpkg_sysgroup_from_gid(filestat->gid);
	if (gr)
		fprintf(out, "%s ", gr->gr_name);
	else if (filestat->gname)
		fprintf(out, "%s ", filestat->gname);
	else
		fprintf(out, "#%d ", filestat->gid);

	fprintf(out, "%o %s\n", filestat->mode & ~S_IFMT, file->name);
}

static void
statdb_write(void)
{
	char *dbname;
	struct atomic_file *dbfile;
	struct fsys_hash_iter *iter;
	struct fsys_namenode *file;

	dbname = dpkg_db_get_path(STATOVERRIDEFILE);
	dbfile = atomic_file_new(dbname, ATOMIC_FILE_BACKUP);
	atomic_file_open(dbfile);

	iter = fsys_hash_iter_new();
	while ((file = fsys_hash_iter_next(iter)))
		statdb_node_print(dbfile->fp, file);
	fsys_hash_iter_free(iter);

	atomic_file_sync(dbfile);
	atomic_file_close(dbfile);
	atomic_file_commit(dbfile);
	atomic_file_free(dbfile);

	dir_sync_path(dpkg_db_get_dir());

	free(dbname);
}

static int
statoverride_add(const char *const *argv)
{
	const char *user = argv[0];
	const char *group = argv[1];
	const char *mode = argv[2];
	const char *path = argv[3];
	char *filename;
	struct file_stat **filestat;

	if (!user || !group || !mode || !path || argv[4])
		badusage(_("--%s needs four arguments"), cipaction->olong);

	if (strchr(path, '\n'))
		badusage(_("path may not contain newlines"));

	ensure_statoverrides(STATDB_PARSE_LAX);

	filename = path_cleanup(path);

	filestat = statdb_node_find(filename);
	if (*filestat != NULL) {
		if (in_force(FORCE_STATOVERRIDE_ADD))
			warning(_("an override for '%s' already exists, "
			          "but --force specified so will be ignored"),
			        filename);
		else
			ohshit(_("an override for '%s' already exists; "
			         "aborting"), filename);
	}

	*filestat = statdb_node_new(user, group, mode);

	if (opt_update) {
		struct stat st;
		struct varbuf realfilename = VARBUF_INIT;

		varbuf_add_str(&realfilename, dpkg_fsys_get_dir());
		varbuf_add_str(&realfilename, filename);

		if (stat(varbuf_str(&realfilename), &st) == 0) {
			(*filestat)->mode |= st.st_mode & S_IFMT;
			statdb_node_apply(varbuf_str(&realfilename), *filestat);
		} else if (opt_verbose) {
			warning(_("--update given but %s does not exist"),
			        varbuf_str(&realfilename));
		}

		varbuf_destroy(&realfilename);
	}

	statdb_write();

	free(filename);

	return 0;
}

static int
statoverride_remove(const char *const *argv)
{
	const char *path = argv[0];
	char *filename;

	if (!path || argv[1])
		badusage(_("--%s needs a single argument"), "remove");

	ensure_statoverrides(STATDB_PARSE_LAX);

	filename = path_cleanup(path);

	if (!statdb_node_remove(filename)) {
		if (opt_verbose)
			warning(_("no override present"));
		if (in_force(FORCE_STATOVERRIDE_DEL))
			return 0;
		else
			return 2;
	}

	if (opt_update && opt_verbose)
		warning(_("--update is useless for --remove"));

	statdb_write();

	free(filename);

	return 0;
}

static int
statoverride_list(const char *const *argv)
{
	struct fsys_hash_iter *iter;
	struct fsys_namenode *file;
	const char *thisarg;
	struct glob_node *glob_list = NULL;
	int ret = 1;

	ensure_statoverrides(STATDB_PARSE_LAX);

	while ((thisarg = *argv++)) {
		char *pattern = path_cleanup(thisarg);

		glob_list_prepend(&glob_list, pattern);
	}
	if (glob_list == NULL)
		glob_list_prepend(&glob_list, m_strdup("*"));

	iter = fsys_hash_iter_new();
	while ((file = fsys_hash_iter_next(iter))) {
		struct glob_node *g;

		for (g = glob_list; g; g = g->next) {
			if (fnmatch(g->pattern, file->name, 0) == 0) {
				statdb_node_print(stdout, file);
				ret = 0;
				break;
			}
		}
	}
	fsys_hash_iter_free(iter);

	glob_list_free(glob_list);

	return ret;
}

static void
set_force_obsolete(const struct cmdinfo *cip, const char *value)
{
	warning(_("deprecated --%s option; use --%s instead"),
	        cip->olong, "force-all");
	set_force(FORCE_ALL);
}

static const struct cmdinfo cmdinfos[] = {
	ACTION("add",    0, act_install,   statoverride_add),
	ACTION("remove", 0, act_remove,    statoverride_remove),
	ACTION("list",   0, act_listfiles, statoverride_list),
	ACTION("help",   '?', act_help,    usage),
	ACTION("version", 0,  act_version, printversion),

	{ "admindir",   0,   1,  NULL,         NULL,      set_admindir, 0 },
	{ "instdir",    0,   1,  NULL,         NULL,      set_instdir,  0 },
	{ "root",       0,   1,  NULL,         NULL,      set_root,     0 },
	{ "quiet",      0,   0,  &opt_verbose, NULL,      NULL, 0       },
	{ "force",      0,   0,  NULL,         NULL,      set_force_obsolete },
	{ "force",      0,   2,  NULL,         NULL,      set_force_option, 1 },
	{ "no-force",   0,   2,  NULL,         NULL,      set_force_option, 0 },
	{ "refuse",     0,   2,  NULL,         NULL,      set_force_option, 0 },
	{ "update",     0,   0,  &opt_update,  NULL,      NULL, 1       },
	{  NULL,        0                                               }
};

int
main(int argc, const char *const *argv)
{
	int ret;

	dpkg_locales_init(PACKAGE);
	dpkg_program_init("dpkg-statoverride");
	set_force_default(FORCE_STATCMD_MASK);
	dpkg_options_parse(&argv, cmdinfos, printforhelp);

	debug(dbg_general, "root=%s admindir=%s", dpkg_fsys_get_dir(), dpkg_db_get_dir());

	if (!cipaction)
		badusage(_("need an action option"));

	ret = cipaction->action(argv);

	dpkg_program_done();
	dpkg_locales_done();

	return ret;
}
