/*
 * dpkg-divert - override a package's version of a file
 *
 * Copyright © 1995 Ian Jackson
 * Copyright © 2000, 2001 Wichert Akkerman
 * Copyright © 2006-2015, 2017-2018 Guillem Jover <guillem@debian.org>
 * Copyright © 2011 Linaro Limited
 * Copyright © 2011 Raphaël Hertzog <hertzog@debian.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
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
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <fcntl.h>
#include <fnmatch.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/debug.h>
#include <dpkg/arch.h>
#include <dpkg/file.h>
#include <dpkg/glob.h>
#include <dpkg/buffer.h>
#include <dpkg/options.h>
#include <dpkg/db-fsys.h>


static const char printforhelp[] = N_(
"Use --help for help about diverting files.");

static bool opt_pkgname_match_any = true;
static const char *opt_pkgname = NULL;
static const char *opt_divertto = NULL;

static int opt_verbose = 1;
static int opt_test = 0;
static int opt_rename = -1;


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
"Usage: %s [<option>...] <command>\n"
"\n"), dpkg_get_progname());

	printf(_(
"Commands:\n"
"  [--add] <file>           add a diversion.\n"
"  --remove <file>          remove the diversion.\n"
"  --list [<glob-pattern>]  show file diversions.\n"
"  --listpackage <file>     show what package diverts the file.\n"
"  --truename <file>        return the diverted file.\n"
"\n"));

	printf(_(
"Options:\n"
"  --package <package>      name of the package whose copy of <file> will not\n"
"                             be diverted.\n"
"  --local                  all packages' versions are diverted.\n"
"  --divert <divert-to>     the name used by other packages' versions.\n"
"  --rename                 actually move the file aside (or back).\n"
"  --no-rename              do not move the file aside (or back) (default).\n"
"  --admindir <directory>   set the directory with the diversions file.\n"
"  --instdir <directory>    set the root directory, but not the admin dir.\n"
"  --root <directory>       set the directory of the root filesystem.\n"
"  --test                   don't do anything, just demonstrate.\n"
"  --quiet                  quiet operation, minimal output.\n"
"  --help                   show this help message.\n"
"  --version                show the version.\n"
"\n"));

	printf(_(
"When adding, default is --local and --divert <original>.distrib.\n"
"When removing, --package or --local and --divert must match if specified.\n"
"Package preinst/postrm scripts should always specify --package and --divert.\n"));

	m_output(stdout, _("<standard output>"));

	return 0;
}

static void
opt_rename_setup(void)
{
	if (opt_rename >= 0)
		return;

	opt_rename = 0;
	warning(_("please specify --no-rename explicitly, the default "
	          "will change to --rename in 1.20.x"));
}

struct file {
	char *name;
	enum {
		FILE_STAT_INVALID,
		FILE_STAT_VALID,
		FILE_STAT_NOFILE,
	} stat_state;
	struct stat stat;
};

static void
file_init(struct file *f, const char *filename)
{
	struct varbuf usefilename = VARBUF_INIT;

	varbuf_add_str(&usefilename, dpkg_fsys_get_dir());
	varbuf_add_str(&usefilename, filename);

	f->name = varbuf_detach(&usefilename);
	f->stat_state = FILE_STAT_INVALID;
}

static void
file_destroy(struct file *f)
{
	free(f->name);
}

static void
file_stat(struct file *f)
{
	int ret;

	if (f->stat_state != FILE_STAT_INVALID)
		return;

	ret = lstat(f->name, &f->stat);
	if (ret && errno != ENOENT)
		ohshite(_("cannot stat file '%s'"), f->name);

	if (ret == 0)
		f->stat_state = FILE_STAT_VALID;
	else
		f->stat_state = FILE_STAT_NOFILE;
}

static void
check_writable_dir(struct file *f)
{
	char *tmpname;
	int tmpfd;

	tmpname = str_fmt("%s%s", f->name, ".dpkg-divert.tmp");

	tmpfd = creat(tmpname, 0600);
	if (tmpfd < 0)
		ohshite(_("error checking '%s'"), f->name);
	close(tmpfd);
	(void)unlink(tmpname);

	free(tmpname);
}

static bool
check_rename(struct file *src, struct file *dst)
{
	file_stat(src);

	/* If the source file is not present and we are not going to do
	 * the rename anyway there's no point in checking any further. */
	if (src->stat_state == FILE_STAT_NOFILE)
		return false;

	file_stat(dst);

	/*
	 * Unfortunately we have to check for write access in both places,
	 * just having +w is not enough, since people do mount things RO,
	 * and we need to fail before we start mucking around with things.
	 * So we open a file with the same name as the diversions but with
	 * an extension that (hopefully) won't overwrite anything. If it
	 * succeeds, we assume a writable filesystem.
	 */

	check_writable_dir(src);
	check_writable_dir(dst);

	if (src->stat_state == FILE_STAT_VALID &&
	    dst->stat_state == FILE_STAT_VALID &&
	    !(src->stat.st_dev == dst->stat.st_dev &&
	      src->stat.st_ino == dst->stat.st_ino))
		ohshit(_("rename involves overwriting '%s' with\n"
		         "  different file '%s', not allowed"),
		        dst->name, src->name);

	return true;
}

static void
file_copy(const char *src, const char *dst)
{
	struct dpkg_error err;
	char *tmp;
	int srcfd, dstfd;

	srcfd = open(src, O_RDONLY);
	if (srcfd < 0)
		ohshite(_("unable to open file '%s'"), src);

	tmp = str_fmt("%s%s", dst, ".dpkg-divert.tmp");
	dstfd = creat(tmp, 0600);
	if (dstfd < 0)
		ohshite(_("unable to create file '%s'"), tmp);

	push_cleanup(cu_filename, ~ehflag_normaltidy, 1, tmp);

	if (fd_fd_copy(srcfd, dstfd, -1, &err) < 0)
		ohshit(_("cannot copy '%s' to '%s': %s"), src, tmp, err.str);

	close(srcfd);

	if (fsync(dstfd))
		ohshite(_("unable to sync file '%s'"), tmp);
	if (close(dstfd))
		ohshite(_("unable to close file '%s'"), tmp);

	file_copy_perms(src, tmp);

	if (rename(tmp, dst) != 0)
		ohshite(_("cannot rename '%s' to '%s'"), tmp, dst);

	free(tmp);

	pop_cleanup(ehflag_normaltidy);
}

static void
file_rename(struct file *src, struct file *dst)
{
	if (src->stat_state == FILE_STAT_NOFILE)
		return;

	if (dst->stat_state == FILE_STAT_VALID) {
		if (unlink(src->name))
			ohshite(_("rename: remove duplicate old link '%s'"),
			        src->name);
	} else {
		if (rename(src->name, dst->name) == 0)
			return;

		/* If a rename didn't work try moving the file instead. */
		file_copy(src->name, dst->name);

		if (unlink(src->name))
			ohshite(_("unable to remove copied source file '%s'"),
			        src->name);
	}
}

static void
diversion_check_filename(const char *filename)
{
	if (filename[0] != '/')
		badusage(_("filename \"%s\" is not absolute"), filename);
	if (strchr(filename, '\n') != NULL)
		badusage(_("file may not contain newlines"));
}

static const char *
diversion_pkg_name(struct fsys_diversion *d)
{
	if (d->pkgset == NULL)
		return ":";
	else
		return d->pkgset->name;
}

static const char *
varbuf_diversion(struct varbuf *str, const char *pkgname,
                 const char *filename, const char *divertto)
{
	if (pkgname == NULL) {
		if (divertto == NULL)
			varbuf_set_fmt(str, _("local diversion of %s"), filename);
		else
			varbuf_set_fmt(str, _("local diversion of %s to %s"),
			              filename, divertto);
	} else {
		if (divertto == NULL)
			varbuf_set_fmt(str, _("diversion of %s by %s"),
			               filename, pkgname);
		else
			varbuf_set_fmt(str, _("diversion of %s to %s by %s"),
			               filename, divertto, pkgname);
	}

	return varbuf_str(str);
}

static const char *
diversion_current(const char *filename)
{
	static struct varbuf str = VARBUF_INIT;

	if (opt_pkgname_match_any) {
		if (opt_divertto == NULL)
			varbuf_set_fmt(&str, _("any diversion of %s"), filename);
		else
			varbuf_set_fmt(&str, _("any diversion of %s to %s"),
			               filename, opt_divertto);
	} else {
		return varbuf_diversion(&str, opt_pkgname, filename, opt_divertto);
	}

	return varbuf_str(&str);
}

static const char *
diversion_describe(struct fsys_diversion *d)
{
	static struct varbuf str = VARBUF_INIT;
	const char *pkgname;
	const char *name_from, *name_to;

	if (d->camefrom) {
		name_from = d->camefrom->name;
		name_to = d->camefrom->divert->useinstead->name;
	} else {
		name_from = d->useinstead->divert->camefrom->name;
		name_to = d->useinstead->name;
	}

	if (d->pkgset == NULL)
		pkgname = NULL;
	else
		pkgname = d->pkgset->name;

	return varbuf_diversion(&str, pkgname, name_from, name_to);
}

static void
divertdb_write(void)
{
	char *dbname;
	struct atomic_file *file;
	struct fsys_hash_iter *iter;
	struct fsys_namenode *namenode;

	dbname = dpkg_db_get_path(DIVERSIONSFILE);

	file = atomic_file_new(dbname, ATOMIC_FILE_BACKUP);
	atomic_file_open(file);

	iter = fsys_hash_iter_new();
	while ((namenode = fsys_hash_iter_next(iter))) {
		struct fsys_diversion *d = namenode->divert;

		if (d == NULL || d->useinstead == NULL)
			continue;

		fprintf(file->fp, "%s\n%s\n%s\n",
		        d->useinstead->divert->camefrom->name,
		        d->useinstead->name,
		        diversion_pkg_name(d));
	}
	fsys_hash_iter_free(iter);

	atomic_file_sync(file);
	atomic_file_close(file);
	atomic_file_commit(file);
	atomic_file_free(file);

	free(dbname);
}

static bool
diversion_is_essential(struct fsys_namenode *namenode)
{
	struct pkginfo *pkg;
	struct pkg_hash_iter *pkg_iter;
	struct fsys_node_pkgs_iter *iter;
	bool essential = false;

	pkg_iter = pkg_hash_iter_new();
	while ((pkg = pkg_hash_iter_next_pkg(pkg_iter))) {
		if (pkg->installed.essential)
			ensure_packagefiles_available(pkg);
	}
	pkg_hash_iter_free(pkg_iter);

	iter = fsys_node_pkgs_iter_new(namenode);
	while ((pkg = fsys_node_pkgs_iter_next(iter))) {
		if (pkg->installed.essential) {
			essential = true;
			break;
		}
	}
	fsys_node_pkgs_iter_free(iter);

	return essential;
}

static bool
diversion_is_owned_by_self(struct pkgset *set, struct fsys_namenode *namenode)
{
	struct pkginfo *pkg;
	struct fsys_node_pkgs_iter *iter;
	bool owned = false;

	if (set == NULL)
		return false;

	for (pkg = &set->pkg; pkg; pkg = pkg->arch_next)
		ensure_packagefiles_available(pkg);

	iter = fsys_node_pkgs_iter_new(namenode);
	while ((pkg = fsys_node_pkgs_iter_next(iter))) {
		if (pkg->set == set) {
			owned = true;
			break;
		}
	}
	fsys_node_pkgs_iter_free(iter);

	return owned;
}

static int
diversion_add(const char *const *argv)
{
	const char *filename = argv[0];
	struct file file_from, file_to;
	struct fsys_diversion *contest, *altname;
	struct fsys_namenode *fnn_from, *fnn_to;
	struct pkgset *pkgset;

	opt_pkgname_match_any = false;
	opt_rename_setup();

	/* Handle filename. */
	if (!filename || argv[1])
		badusage(_("--%s needs a single argument"), cipaction->olong);

	diversion_check_filename(filename);

	modstatdb_open(msdbrw_readonly);
	ensure_diversions();

	file_init(&file_from, filename);
	file_stat(&file_from);

	if (file_from.stat_state == FILE_STAT_VALID &&
	    S_ISDIR(file_from.stat.st_mode))
		badusage(_("cannot divert directories"));

	fnn_from = fsys_hash_find_node(filename, FHFF_NONE);

	/* Handle divertto. */
	if (opt_divertto == NULL)
		opt_divertto = str_fmt("%s.distrib", filename);

	if (strcmp(filename, opt_divertto) == 0)
		badusage(_("cannot divert file '%s' to itself"), filename);

	file_init(&file_to, opt_divertto);

	fnn_to = fsys_hash_find_node(opt_divertto, FHFF_NONE);

	/* Handle package name. */
	if (opt_pkgname == NULL)
		pkgset = NULL;
	else
		pkgset = pkg_hash_find_set(opt_pkgname);

	/* Check we are not stomping over an existing diversion. */
	if (fnn_from->divert || fnn_to->divert) {
		if (fnn_to->divert && fnn_to->divert->camefrom &&
		    strcmp(fnn_to->divert->camefrom->name, filename) == 0 &&
		    fnn_from->divert && fnn_from->divert->useinstead &&
		    strcmp(fnn_from->divert->useinstead->name, opt_divertto) == 0 &&
		    fnn_from->divert->pkgset == pkgset) {
			if (opt_verbose > 0)
				printf(_("Leaving '%s'\n"),
				       diversion_describe(fnn_from->divert));

			file_destroy(&file_from);
			file_destroy(&file_to);

			modstatdb_shutdown();

			return 0;
		}

		ohshit(_("'%s' clashes with '%s'"),
		       diversion_current(filename),
		       fnn_from->divert ?
		       diversion_describe(fnn_from->divert) :
		       diversion_describe(fnn_to->divert));
	}

	/* Create new diversion. */
	contest = nfmalloc(sizeof(*contest));
	altname = nfmalloc(sizeof(*altname));

	altname->camefrom = fnn_from;
	altname->camefrom->divert = contest;
	altname->useinstead = NULL;
	altname->pkgset = pkgset;

	contest->useinstead = fnn_to;
	contest->useinstead->divert = altname;
	contest->camefrom = NULL;
	contest->pkgset = pkgset;

	/* Update database and file system if needed. */
	if (opt_verbose > 0)
		printf(_("Adding '%s'\n"), diversion_describe(contest));
	if (opt_rename)
		opt_rename = check_rename(&file_from, &file_to);
	/* Check we are not renaming a file owned by the diverting pkgset. */
	if (opt_rename && diversion_is_owned_by_self(pkgset, fnn_from)) {
		if (opt_verbose > 0)
			printf(_("Ignoring request to rename file '%s' "
			         "owned by diverting package '%s'\n"),
			       filename, pkgset->name);
		opt_rename = false;
	}
	if (opt_rename && diversion_is_essential(fnn_from))
		warning(_("diverting file '%s' from an Essential package with "
		          "rename is dangerous, use --no-rename"), filename);
	if (!opt_test) {
		divertdb_write();
		if (opt_rename)
			file_rename(&file_from, &file_to);
	}

	file_destroy(&file_from);
	file_destroy(&file_to);

	modstatdb_shutdown();

	return 0;
}

static bool
diversion_is_shared(struct pkgset *set, struct fsys_namenode *namenode)
{
	const char *archname;
	struct pkginfo *pkg;
	struct dpkg_arch *arch;
	struct fsys_node_pkgs_iter *iter;
	bool shared = false;

	if (set == NULL)
		return false;

	archname = getenv("DPKG_MAINTSCRIPT_ARCH");
	arch = dpkg_arch_find(archname);
	if (arch->type == DPKG_ARCH_NONE || arch->type == DPKG_ARCH_EMPTY)
		return false;

	for (pkg = &set->pkg; pkg; pkg = pkg->arch_next)
		ensure_packagefiles_available(pkg);

	iter = fsys_node_pkgs_iter_new(namenode);
	while ((pkg = fsys_node_pkgs_iter_next(iter))) {
		if (pkg->set == set && pkg->installed.arch != arch) {
			shared = true;
			break;
		}
	}
	fsys_node_pkgs_iter_free(iter);

	return shared;
}

static int
diversion_remove(const char *const *argv)
{
	const char *filename = argv[0];
	struct fsys_namenode *namenode;
	struct fsys_diversion *contest, *altname;
	struct file file_from, file_to;
	struct pkgset *pkgset;

	opt_rename_setup();

	if (!filename || argv[1])
		badusage(_("--%s needs a single argument"), cipaction->olong);

	diversion_check_filename(filename);

	modstatdb_open(msdbrw_readonly);
	ensure_diversions();

	namenode = fsys_hash_find_node(filename, FHFF_NO_NEW);

	if (namenode == NULL || namenode->divert == NULL ||
	    namenode->divert->useinstead == NULL) {
		if (opt_verbose > 0)
			printf(_("No diversion '%s', none removed.\n"),
			       diversion_current(filename));
		modstatdb_shutdown();
		return 0;
	}

	if (opt_pkgname == NULL)
		pkgset = NULL;
	else
		pkgset = pkg_hash_find_set(opt_pkgname);

	contest = namenode->divert;
	altname = contest->useinstead->divert;

	if (opt_divertto != NULL &&
	    strcmp(opt_divertto, contest->useinstead->name) != 0)
		ohshit(_("mismatch on divert-to\n"
		         "  when removing '%s'\n"
		         "  found '%s'"),
		       diversion_current(filename),
		       diversion_describe(contest));

	if (!opt_pkgname_match_any && pkgset != contest->pkgset)
		ohshit(_("mismatch on package\n"
		         "  when removing '%s'\n"
		         "  found '%s'"),
		       diversion_current(filename),
		       diversion_describe(contest));

	/* Ignore removal request if the diverted file is still owned
	 * by another package in the same set. */
	if (diversion_is_shared(pkgset, namenode)) {
		if (opt_verbose > 0)
			printf(_("Ignoring request to remove shared diversion '%s'.\n"),
			       diversion_describe(contest));
		modstatdb_shutdown();
		return 0;
	}

	if (opt_verbose > 0)
		printf(_("Removing '%s'\n"), diversion_describe(contest));

	file_init(&file_from, altname->camefrom->name);
	file_init(&file_to, contest->useinstead->name);

	/* Remove entries from database. */
	contest->useinstead->divert = NULL;
	altname->camefrom->divert = NULL;

	if (opt_rename)
		opt_rename = check_rename(&file_to, &file_from);
	if (opt_rename && !opt_test)
		file_rename(&file_to, &file_from);

	if (!opt_test)
		divertdb_write();

	file_destroy(&file_from);
	file_destroy(&file_to);

	modstatdb_shutdown();

	return 0;
}

static int
diversion_list(const char *const *argv)
{
	struct fsys_hash_iter *iter;
	struct fsys_namenode *namenode;
	struct glob_node *glob_list = NULL;
	const char *pattern;

	modstatdb_open(msdbrw_readonly);
	ensure_diversions();

	while ((pattern = *argv++))
		glob_list_prepend(&glob_list, m_strdup(pattern));

	if (glob_list == NULL)
		glob_list_prepend(&glob_list, m_strdup("*"));

	iter = fsys_hash_iter_new();
	while ((namenode = fsys_hash_iter_next(iter))) {
		struct glob_node *g;
		struct fsys_diversion *contest = namenode->divert;
		struct fsys_diversion *altname;
		const char *pkgname;

		if (contest == NULL || contest->useinstead == NULL)
			continue;

		altname = contest->useinstead->divert;

		pkgname = diversion_pkg_name(contest);

		for (g = glob_list; g; g = g->next) {
			if (fnmatch(g->pattern, pkgname, 0) == 0 ||
			    fnmatch(g->pattern, contest->useinstead->name, 0) == 0 ||
			    fnmatch(g->pattern, altname->camefrom->name, 0) == 0) {
				printf("%s\n", diversion_describe(contest));
				break;
			}
		}
	}
	fsys_hash_iter_free(iter);

	glob_list_free(glob_list);

	modstatdb_shutdown();

	return 0;
}

static int
diversion_truename(const char *const *argv)
{
	const char *filename = argv[0];
	struct fsys_namenode *namenode;

	if (!filename || argv[1])
		badusage(_("--%s needs a single argument"), cipaction->olong);

	diversion_check_filename(filename);

	modstatdb_open(msdbrw_readonly);
	ensure_diversions();

	namenode = fsys_hash_find_node(filename, FHFF_NO_NEW);

	/* Print the given name if file is not diverted. */
	if (namenode && namenode->divert && namenode->divert->useinstead)
		printf("%s\n", namenode->divert->useinstead->name);
	else
		printf("%s\n", filename);

	modstatdb_shutdown();

	return 0;
}

static int
diversion_listpackage(const char *const *argv)
{
	const char *filename = argv[0];
	struct fsys_namenode *namenode;

	if (!filename || argv[1])
		badusage(_("--%s needs a single argument"), cipaction->olong);

	diversion_check_filename(filename);

	modstatdb_open(msdbrw_readonly);
	ensure_diversions();

	namenode = fsys_hash_find_node(filename, FHFF_NO_NEW);

	/* Print nothing if file is not diverted. */
	if (namenode == NULL || namenode->divert == NULL)
		return 0;

	if (namenode->divert->pkgset == NULL)
		/* Indicate package is local using something not in package
		 * namespace. */
		printf("LOCAL\n");
	else
		printf("%s\n", namenode->divert->pkgset->name);

	modstatdb_shutdown();

	return 0;
}

static void
set_package(const struct cmdinfo *cip, const char *value)
{
	opt_pkgname_match_any = false;

	/* If value is NULL we are being called from --local. */
	opt_pkgname = value;

	if (opt_pkgname && strchr(opt_pkgname, '\n') != NULL)
		badusage(_("package may not contain newlines"));
}

static void
set_divertto(const struct cmdinfo *cip, const char *value)
{
	opt_divertto = value;

	if (opt_divertto[0] != '/')
		badusage(_("filename \"%s\" is not absolute"), opt_divertto);
	if (strchr(opt_divertto, '\n') != NULL)
		badusage(_("divert-to may not contain newlines"));
}

static const struct cmdinfo cmdinfo_add =
	ACTION("add",         0, 0, diversion_add);

static const struct cmdinfo cmdinfos[] = {
	ACTION("add",         0, 0, diversion_add),
	ACTION("remove",      0, 0, diversion_remove),
	ACTION("list",        0, 0, diversion_list),
	ACTION("listpackage", 0, 0, diversion_listpackage),
	ACTION("truename",    0, 0, diversion_truename),
	ACTION("help",        '?', 0, usage),
	ACTION("version",     0,   0, printversion),

	{ "admindir",   0,   1,  NULL,         NULL,      set_admindir, 0 },
	{ "instdir",    0,   1,  NULL,         NULL,      set_instdir,  0 },
	{ "root",       0,   1,  NULL,         NULL,      set_root,     0 },
	{ "divert",     0,   1,  NULL,         NULL,      set_divertto  },
	{ "package",    0,   1,  NULL,         NULL,      set_package   },
	{ "local",      0,   0,  NULL,         NULL,      set_package   },
	{ "quiet",      0,   0,  &opt_verbose, NULL,      NULL, 0       },
	{ "rename",     0,   0,  &opt_rename,  NULL,      NULL, 1       },
	{ "no-rename",  0,   0,  &opt_rename,  NULL,      NULL, 0       },
	{ "test",       0,   0,  &opt_test,    NULL,      NULL, 1       },
	{  NULL,        0                                               }
};

int
main(int argc, const char * const *argv)
{
	const char *env_pkgname;
	int ret;

	dpkg_locales_init(PACKAGE);
	dpkg_program_init("dpkg-divert");
	dpkg_options_parse(&argv, cmdinfos, printforhelp);

	debug(dbg_general, "root=%s admindir=%s", dpkg_fsys_get_dir(), dpkg_db_get_dir());

	env_pkgname = getenv("DPKG_MAINTSCRIPT_PACKAGE");
	if (opt_pkgname_match_any && env_pkgname)
		set_package(NULL, env_pkgname);

	if (!cipaction)
		setaction(&cmdinfo_add, NULL);

	ret = cipaction->action(argv);

	dpkg_program_done();
	dpkg_locales_done();

	return ret;
}
