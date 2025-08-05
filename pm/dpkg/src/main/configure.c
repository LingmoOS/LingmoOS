/*
 * dpkg - main program for package management
 * configure.c - configure packages
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 1999, 2002 Wichert Akkerman <wichert@deephackmode.org>
 * Copyright © 2007-2015 Guillem Jover <guillem@debian.org>
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
#include <sys/wait.h>

#include <errno.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <termios.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/macros.h>
#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg.h>
#include <dpkg/string.h>
#include <dpkg/buffer.h>
#include <dpkg/file.h>
#include <dpkg/path.h>
#include <dpkg/subproc.h>
#include <dpkg/command.h>
#include <dpkg/pager.h>
#include <dpkg/triglib.h>
#include <dpkg/db-fsys.h>

#include "main.h"

enum DPKG_ATTR_ENUM_FLAGS conffopt {
	CFOF_PROMPT		= DPKG_BIT(0),
	CFOF_KEEP		= DPKG_BIT(1),
	CFOF_INSTALL		= DPKG_BIT(2),
	CFOF_BACKUP		= DPKG_BIT(3),
	CFOF_NEW_CONFF		= DPKG_BIT(4),
	CFOF_IS_NEW		= DPKG_BIT(5),
	CFOF_IS_OLD		= DPKG_BIT(6),
	CFOF_USER_DEL		= DPKG_BIT(7),

	CFO_KEEP		= CFOF_KEEP,
	CFO_IDENTICAL		= CFOF_KEEP,
	CFO_INSTALL		= CFOF_INSTALL,
	CFO_NEW_CONFF		= CFOF_NEW_CONFF | CFOF_INSTALL,
	CFO_PROMPT		= CFOF_PROMPT,
	CFO_PROMPT_KEEP		= CFOF_PROMPT | CFOF_KEEP,
	CFO_PROMPT_INSTALL	= CFOF_PROMPT | CFOF_INSTALL,
};

static int conffoptcells[2][2] = {
	/* Distro !edited. */	/* Distro edited. */
	{ CFO_KEEP,		CFO_INSTALL },		/* User !edited. */
	{ CFO_KEEP,		CFO_PROMPT_KEEP },	/* User edited. */
};

static int
show_prompt(const char *cfgfile, const char *realold, const char *realnew,
            int useredited, int distedited, enum conffopt what)
{
	const char *s;
	int c, cc;

	/* Flush the terminal's input in case the user involuntarily
	 * typed some characters. */
	tcflush(STDIN_FILENO, TCIFLUSH);

	fputs("\n", stderr);
	if (strcmp(cfgfile, realold) == 0)
		fprintf(stderr, _("Configuration file '%s'\n"), cfgfile);
	else
		fprintf(stderr, _("Configuration file '%s' (actually '%s')\n"),
		        cfgfile, realold);

	if (what & CFOF_IS_NEW) {
		fprintf(stderr,
		        _(" ==> File on system created by you or by a script.\n"
		          " ==> File also in package provided by package maintainer.\n"));
	} else {
		fprintf(stderr, !useredited ?
		        _("     Not modified since installation.\n") :
		        !(what & CFOF_USER_DEL) ?
		        _(" ==> Modified (by you or by a script) since installation.\n") :
		        _(" ==> Deleted (by you or by a script) since installation.\n"));

		fprintf(stderr, distedited ?
		        _(" ==> Package distributor has shipped an updated version.\n") :
		        _("     Version in package is the same as at last installation.\n"));
	}

	/* No --force-confdef but a forcible situation. */
	/* TODO: check if this condition can not be simplified to
	 *       just !in_force(FORCE_CONFF_DEF) */
	if (!(in_force(FORCE_CONFF_DEF) && (what & (CFOF_INSTALL | CFOF_KEEP)))) {
		if (in_force(FORCE_CONFF_NEW)) {
			fprintf(stderr,
			        _(" ==> Using new file as you requested.\n"));
			return 'y';
		} else if (in_force(FORCE_CONFF_OLD)) {
			fprintf(stderr,
			        _(" ==> Using current old file as you requested.\n"));
			return 'n';
		}
	}

	/* Force the default action (if there is one. */
	if (in_force(FORCE_CONFF_DEF)) {
		if (what & CFOF_KEEP) {
			fprintf(stderr,
			        _(" ==> Keeping old config file as default.\n"));
			return 'n';
		} else if (what & CFOF_INSTALL) {
			fprintf(stderr,
			        _(" ==> Using new config file as default.\n"));
			return 'y';
		}
	}

	fprintf(stderr,
	        _("   What would you like to do about it ?  Your options are:\n"
	          "    Y or I  : install the package maintainer's version\n"
	          "    N or O  : keep your currently-installed version\n"
	          "      D     : show the differences between the versions\n"
	          "      Z     : start a shell to examine the situation\n"));

	if (what & CFOF_KEEP)
		fprintf(stderr,
		        _(" The default action is to keep your current version.\n"));
	else if (what & CFOF_INSTALL)
		fprintf(stderr,
		        _(" The default action is to install the new version.\n"));

	s = path_basename(cfgfile);
	fprintf(stderr, "*** %s (Y/I/N/O/D/Z) %s ? ", s,
	        (what & CFOF_KEEP) ? _("[default=N]") :
	        (what & CFOF_INSTALL) ? _("[default=Y]") :
	        _("[no default]"));

	if (ferror(stderr))
		ohshite(_("error writing to stderr, discovered before conffile prompt"));

	cc = 0;
	while ((c = getchar()) != EOF && c != '\n')
		if (!isspace(c) && !cc)
			cc = tolower(c);

	if (c == EOF) {
		if (ferror(stdin))
			ohshite(_("read error on stdin at conffile prompt"));
		ohshit(_("end of file on stdin at conffile prompt"));
	}

	if (!cc) {
		if (what & CFOF_KEEP)
			return 'n';
		else if (what & CFOF_INSTALL)
			return 'y';
	}

	return cc;
}

/**
 * Show a diff between two files.
 *
 * @param old The path to the old file.
 * @param new The path to the new file.
 */
static void
show_diff(const char *old, const char *new)
{
	struct pager *pager;
	pid_t pid;

	pager = pager_spawn(_("conffile difference visualizer"));

	pid = subproc_fork();
	if (!pid) {
		/* Child process. */
		struct command cmd;

		command_init(&cmd, DIFF, _("conffile difference visualizer"));
		command_add_arg(&cmd, DIFF);
		command_add_arg(&cmd, "-Nu");
		command_add_arg(&cmd, old);
		command_add_arg(&cmd, new);
		command_exec(&cmd);
	}

	/* Parent process. */
	subproc_reap(pid, _("conffile difference visualizer"), SUBPROC_NOCHECK);
	pager_reap(pager);
}

/**
 * Spawn a new shell.
 *
 * Create a subprocess and execute a shell to allow the user to manually
 * solve the conffile conflict.
 *
 * @param confold The path to the old conffile.
 * @param confnew The path to the new conffile.
 */
static void
spawn_shell(const char *confold, const char *confnew)
{
	pid_t pid;

	fputs(_("Useful environment variables:\n"), stderr);
	fputs(" - DPKG_SHELL_REASON\n", stderr);
	fputs(" - DPKG_CONFFILE_OLD\n", stderr);
	fputs(" - DPKG_CONFFILE_NEW\n", stderr);
	fputs(_("Type 'exit' when you're done.\n"), stderr);

	pid = subproc_fork();
	if (!pid) {
		/* Set useful variables for the user. */
		setenv("DPKG_SHELL_REASON", "conffile-prompt", 1);
		setenv("DPKG_CONFFILE_OLD", confold, 1);
		setenv("DPKG_CONFFILE_NEW", confnew, 1);

		command_shell(NULL, _("conffile shell"));
	}

	/* Parent process. */
	subproc_reap(pid, _("conffile shell"), SUBPROC_NOCHECK);
}

/**
 * Prompt the user for how to resolve a conffile conflict.
 *
 * When encountering a conffile conflict during configuration, the user will
 * normally be presented with a textual menu of possible actions. This
 * behavior is modified via various --force flags and perhaps on whether
 * or not a terminal is available to do the prompting.
 *
 * @param pkg The package owning the conffile.
 * @param cfgfile The path to the old conffile.
 * @param realold The path to the old conffile, dereferenced in case of a
 *        symlink, otherwise equal to cfgfile.
 * @param realnew The path to the new conffile, dereferenced in case of a
 *        symlink).
 * @param useredited A flag to indicate whether the file has been edited
 *        locally. Set to nonzero to indicate that the file has been modified.
 * @param distedited A flag to indicate whether the file has been updated
 *        between package versions. Set to nonzero to indicate that the file
 *        has been updated.
 * @param what Hints on what action should be taken by default.
 *
 * @return The action which should be taken based on user input and/or the
 *         default actions as configured by cmdline/configuration options.
 */
static enum conffopt
promptconfaction(struct pkginfo *pkg, const char *cfgfile,
                 const char *realold, const char *realnew,
                 int useredited, int distedited, enum conffopt what)
{
	int cc;

	if (!(what & CFOF_PROMPT))
		return what;

	statusfd_send("status: %s : %s : '%s' '%s' %i %i ",
	              cfgfile, "conffile-prompt",
	              realold, realnew, useredited, distedited);

	do {
		cc = show_prompt(cfgfile, realold, realnew,
		                 useredited, distedited, what);

		/* FIXME: Say something if silently not install. */
		if (cc == 'd')
			show_diff(realold, realnew);

		if (cc == 'z')
			spawn_shell(realold, realnew);
	} while (!strchr("yino", cc));

	log_message("conffile %s %s", cfgfile,
	            (cc == 'i' || cc == 'y') ? "install" : "keep");

	what &= CFOF_USER_DEL;

	switch (cc) {
	case 'i':
	case 'y':
		what |= CFOF_INSTALL | CFOF_BACKUP;
		break;

	case 'n':
	case 'o':
		what |= CFOF_KEEP | CFOF_BACKUP;
		break;

	default:
		internerr("unknown response '%d'", cc);
	}

	return what;
}

/**
 * Configure the ghost conffile instance.
 *
 * When the first instance of a package set is configured, the *.dpkg-new
 * files gets installed into their destination, which makes configuration of
 * conffiles from subsequent package instances be skipped along with updates
 * to the Conffiles field hash.
 *
 * In case the conffile has already been processed, sync the hash from an
 * already configured package instance conffile.
 *
 * @param pkg	The current package being configured.
 * @param conff	The current conffile being configured.
 */
static void
deferred_configure_ghost_conffile(struct pkginfo *pkg, struct conffile *conff)
{
	struct pkginfo *otherpkg;
	struct conffile *otherconff;

	for (otherpkg = &pkg->set->pkg; otherpkg; otherpkg = otherpkg->arch_next) {
		if (otherpkg == pkg)
			continue;
		if (otherpkg->status <= PKG_STAT_HALFCONFIGURED)
			continue;

		for (otherconff = otherpkg->installed.conffiles; otherconff;
		     otherconff = otherconff->next) {
			if (conffile_is_disappearing(otherconff))
				continue;

			/* Check if we need to propagate the new hash from
			 * an already processed conffile in another package
			 * instance. */
			if (strcmp(otherconff->name, conff->name) == 0) {
				conff->hash = otherconff->hash;
				modstatdb_note(pkg);
				return;
			}
		}
	}
}

static void
deferred_configure_conffile(struct pkginfo *pkg, struct conffile *conff)
{
	struct fsys_namenode *usenode;
	char currenthash[MD5HASHLEN + 1], newdisthash[MD5HASHLEN + 1];
	int useredited, distedited;
	enum conffopt what;
	struct stat stab;
	struct varbuf cdr = VARBUF_INIT;
	struct varbuf cdr_new = VARBUF_INIT;
	struct varbuf cdr_old = VARBUF_INIT;
	struct varbuf cdr_dist = VARBUF_INIT;
	int rc;

	usenode = namenodetouse(fsys_hash_find_node(conff->name, FHFF_NO_COPY),
                                pkg, &pkg->installed);

	rc = conffderef(pkg, &cdr, usenode->name);
	if (rc < 0) {
		conff->hash = EMPTYHASHFLAG;
		return;
	}
	md5hash(pkg, currenthash, cdr.buf);

	varbuf_set_varbuf(&cdr_new, &cdr);
	varbuf_add_str(&cdr_new, DPKGNEWEXT);
	varbuf_set_varbuf(&cdr_old, &cdr);
	varbuf_add_str(&cdr_old, DPKGOLDEXT);
	varbuf_set_varbuf(&cdr_dist, &cdr);
	varbuf_add_str(&cdr_dist, DPKGDISTEXT);

	/* If the .dpkg-new file is no longer there, ignore this one. */
	if (lstat(cdr_new.buf, &stab)) {
		if (errno == ENOENT) {
			/* But, sync the conffile hash value from another
			 * package set instance. */
			deferred_configure_ghost_conffile(pkg, conff);
			return;
		}
		ohshite(_("unable to stat new distributed conffile '%.250s'"),
		        cdr_new.buf);
	}
	md5hash(pkg, newdisthash, cdr_new.buf);

	/* Copy the permissions from the installed version to the new
	 * distributed version. */
	if (!stat(cdr.buf, &stab))
		file_copy_perms(cdr.buf, cdr_new.buf);
	else if (errno != ENOENT)
		ohshite(_("unable to stat current installed conffile '%.250s'"),
		        cdr.buf);

	/* Select what to do. */
	if (strcmp(currenthash, newdisthash) == 0) {
		/* They're both the same so there's no point asking silly
		 * questions. */
		useredited = -1;
		distedited = -1;
		what = CFO_IDENTICAL;
	} else if (strcmp(currenthash, NONEXISTENTFLAG) == 0 && in_force(FORCE_CONFF_MISS)) {
		fprintf(stderr,
		        _("\n"
		          "Configuration file '%s', does not exist on system.\n"
		          "Installing new config file as you requested.\n"),
		        usenode->name);
		what = CFO_NEW_CONFF;
		useredited = -1;
		distedited = -1;
	} else if (strcmp(conff->hash, NEWCONFFILEFLAG) == 0) {
		if (strcmp(currenthash, NONEXISTENTFLAG) == 0) {
			what = CFO_NEW_CONFF;
			useredited = -1;
			distedited = -1;
		} else {
			useredited = 1;
			distedited = 1;
			what = conffoptcells[useredited][distedited] |
			       CFOF_IS_NEW;
		}
	} else {
		useredited = strcmp(conff->hash, currenthash) != 0;
		distedited = strcmp(conff->hash, newdisthash) != 0;

		if (in_force(FORCE_CONFF_ASK) && useredited)
			what = CFO_PROMPT_KEEP;
		else
			what = conffoptcells[useredited][distedited];

		if (strcmp(currenthash, NONEXISTENTFLAG) == 0)
			what |= CFOF_USER_DEL;
	}

	debug(dbg_conff,
	      "deferred_configure '%s' (= '%s') useredited=%d distedited=%d what=%o",
	      usenode->name, cdr.buf, useredited, distedited, what);

	what = promptconfaction(pkg, usenode->name, cdr.buf, cdr_new.buf,
	                        useredited, distedited, what);

	switch (what & ~(CFOF_IS_NEW | CFOF_USER_DEL)) {
	case CFO_KEEP | CFOF_BACKUP:
		if (unlink(cdr_old.buf) && errno != ENOENT)
			warning(_("%s: failed to remove old backup '%.250s': %s"),
			        pkg_name(pkg, pnaw_nonambig), cdr_old.buf,
			        strerror(errno));

		trig_path_activate(usenode, pkg);
		if (rename(cdr_new.buf, cdr_dist.buf))
			warning(_("%s: failed to rename '%.250s' to '%.250s': %s"),
			        pkg_name(pkg, pnaw_nonambig), cdr_new.buf, cdr_dist.buf,
			        strerror(errno));
		break;
	case CFO_KEEP:
		if (unlink(cdr_new.buf))
			warning(_("%s: failed to remove '%.250s': %s"),
			        pkg_name(pkg, pnaw_nonambig), cdr_new.buf,
			        strerror(errno));
		break;
	case CFO_INSTALL | CFOF_BACKUP:
		if (unlink(cdr_dist.buf) && errno != ENOENT)
			warning(_("%s: failed to remove old distributed version '%.250s': %s"),
			        pkg_name(pkg, pnaw_nonambig), cdr_dist.buf,
			        strerror(errno));
		if (unlink(cdr_old.buf) && errno != ENOENT)
			warning(_("%s: failed to remove '%.250s' (before overwrite): %s"),
			        pkg_name(pkg, pnaw_nonambig), cdr_old.buf,
			        strerror(errno));
		if (!(what & CFOF_USER_DEL))
			if (link(cdr.buf, cdr_old.buf))
				warning(_("%s: failed to link '%.250s' to '%.250s': %s"),
				        pkg_name(pkg, pnaw_nonambig), cdr.buf,
				        cdr_old.buf, strerror(errno));
		/* Fall through. */
	case CFO_INSTALL:
		printf(_("Installing new version of config file %s ...\n"),
		       usenode->name);
		/* Fall through. */
	case CFO_NEW_CONFF:
		trig_path_activate(usenode, pkg);
		if (rename(cdr_new.buf, cdr.buf))
			ohshite(_("unable to install '%.250s' as '%.250s'"),
			        cdr_new.buf, cdr.buf);
		break;
	default:
		internerr("unknown conffopt '%d'", what);
	}

	conff->hash = nfstrsave(newdisthash);
	modstatdb_note(pkg);

	varbuf_destroy(&cdr);
	varbuf_destroy(&cdr_new);
	varbuf_destroy(&cdr_old);
	varbuf_destroy(&cdr_dist);
}

/**
 * Process the deferred configure package.
 *
 * @param pkg The package to act on.
 */
void
deferred_configure(struct pkginfo *pkg)
{
	struct varbuf aemsgs = VARBUF_INIT;
	struct conffile *conff;
	struct pkginfo *otherpkg;
	enum dep_check ok;

	if (pkg->status == PKG_STAT_NOTINSTALLED)
		ohshit(_("no package named '%s' is installed, cannot configure"),
		       pkg_name(pkg, pnaw_nonambig));
	if (pkg->status == PKG_STAT_INSTALLED)
		ohshit(_("package %.250s is already installed and configured"),
		       pkg_name(pkg, pnaw_nonambig));
	if (pkg->status != PKG_STAT_UNPACKED &&
	    pkg->status != PKG_STAT_HALFCONFIGURED)
		ohshit(_("package %.250s is not ready for configuration\n"
		         " cannot configure (current status '%.250s')"),
		       pkg_name(pkg, pnaw_nonambig),
		       pkg_status_name(pkg));

	for (otherpkg = &pkg->set->pkg; otherpkg; otherpkg = otherpkg->arch_next) {
		if (otherpkg == pkg)
			continue;
		if (otherpkg->status <= PKG_STAT_CONFIGFILES)
			continue;

		if (otherpkg->status < PKG_STAT_UNPACKED)
			ohshit(_("package %s cannot be configured because "
			         "%s is not ready (current status '%s')"),
			       pkg_name(pkg, pnaw_always),
			       pkg_name(otherpkg, pnaw_always),
			       pkg_status_name(otherpkg));

		if (dpkg_version_compare(&pkg->installed.version,
		                         &otherpkg->installed.version))
			ohshit(_("package %s (%s) cannot be configured because "
			         "%s is at a different version (%s)"),
			       pkg_name(pkg, pnaw_always),
			       versiondescribe(&pkg->installed.version,
			                       vdew_nonambig),
			       pkg_name(otherpkg, pnaw_always),
			       versiondescribe(&otherpkg->installed.version,
			                       vdew_nonambig));
	}

	if (dependtry >= DEPEND_TRY_CYCLES)
		if (findbreakcycle(pkg))
			sincenothing = 0;

	ok = dependencies_ok(pkg, NULL, &aemsgs);
	if (ok == DEP_CHECK_DEFER) {
		varbuf_destroy(&aemsgs);
		ensure_package_clientdata(pkg);
		pkg->clientdata->istobe = PKG_ISTOBE_INSTALLNEW;
		enqueue_package(pkg);
		return;
	}

	trigproc_reset_cycle();

	/*
	 * At this point removal from the queue is confirmed. This
	 * represents irreversible progress wrt trigger cycles. Only
	 * packages in PKG_STAT_UNPACKED are automatically added to the
	 * configuration queue, and during configuration and trigger
	 * processing new packages can't enter into unpacked.
	 */

	ok = breakses_ok(pkg, &aemsgs) ? ok : DEP_CHECK_HALT;
	if (ok == DEP_CHECK_HALT) {
		sincenothing = 0;
		notice(_("dependency problems prevent configuration of %s:\n%s"),
		       pkg_name(pkg, pnaw_nonambig), varbuf_str(&aemsgs));
		varbuf_destroy(&aemsgs);
		ohshit(_("dependency problems - leaving unconfigured"));
	} else if (aemsgs.used) {
		notice(_("%s: dependency problems, but configuring anyway as you requested:\n%s"),
		       pkg_name(pkg, pnaw_nonambig), varbuf_str(&aemsgs));
	}
	varbuf_destroy(&aemsgs);
	sincenothing = 0;

	if (pkg->eflag & PKG_EFLAG_REINSTREQ)
		forcibleerr(FORCE_REMOVE_REINSTREQ,
		            _("package is in a very bad inconsistent state; you should\n"
		              " reinstall it before attempting configuration"));

	printf(_("Setting up %s (%s) ...\n"), pkg_name(pkg, pnaw_nonambig),
	       versiondescribe(&pkg->installed.version, vdew_nonambig));
	log_action("configure", pkg, &pkg->installed);

	trig_activate_packageprocessing(pkg);

	if (!f_act) {
		pkg_set_status(pkg, PKG_STAT_INSTALLED);
		ensure_package_clientdata(pkg);
		pkg->clientdata->istobe = PKG_ISTOBE_NORMAL;
		return;
	}

	if (pkg->status == PKG_STAT_UNPACKED) {
		debug(dbg_general, "deferred_configure updating conffiles");
		/* This will not do at all the right thing with overridden
		 * conffiles or conffiles that are the ‘target’ of an override;
		 * all the references here would be to the ‘contested’
		 * filename, and in any case there'd only be one hash for both
		 * ‘versions’ of the conffile.
		 *
		 * Overriding conffiles is a silly thing to do anyway :-). */

		modstatdb_note(pkg);

		/* On entry, the ‘new’ version of each conffile has been
		 * unpacked as ‘*.dpkg-new’, and the ‘installed’ version is
		 * as-yet untouched in ‘*’. The hash of the ‘old distributed’
		 * version is in the conffiles data for the package. If
		 * ‘*.dpkg-new’ no longer exists we assume that we've
		 * already processed this one. */
		for (conff = pkg->installed.conffiles; conff; conff = conff->next) {
			if (conffile_is_disappearing(conff))
				continue;
			deferred_configure_conffile(pkg, conff);
		}

		pkg_set_status(pkg, PKG_STAT_HALFCONFIGURED);
	}

	if (pkg->status != PKG_STAT_HALFCONFIGURED)
		internerr("package %s in state %s, instead of half-configured",
		          pkg_name(pkg, pnaw_always), pkg_status_name(pkg));

	modstatdb_note(pkg);

	maintscript_postinst(pkg, "configure",
	                     dpkg_version_is_informative(&pkg->configversion) ?
	                     versiondescribe(&pkg->configversion,
	                                     vdew_nonambig) : "",
	                     NULL);

	pkg_reset_eflags(pkg);
	pkg->trigpend_head = NULL;
	post_postinst_tasks(pkg, PKG_STAT_INSTALLED);
}

/**
 * Dereference a file by following all possibly used symlinks.
 *
 * @param[in] pkg The package to act on.
 * @param[out] result The dereference conffile path.
 * @param[in] in The conffile path to dereference.
 *
 * @return An error code for the operation.
 * @retval 0 Everything went ok.
 * @retval -1 Otherwise.
 */
int
conffderef(struct pkginfo *pkg, struct varbuf *result, const char *in)
{
	static struct varbuf target = VARBUF_INIT;
	struct stat stab;
	int loopprotect;

	varbuf_set_str(result, dpkg_fsys_get_dir());
	varbuf_add_str(result, in);

	loopprotect = 0;

	for (;;) {
		debug(dbg_conffdetail, "conffderef in='%s' current working='%s'",
		      in, result->buf);
		if (lstat(result->buf, &stab)) {
			if (errno != ENOENT)
				warning(_("%s: unable to stat config file '%s'\n"
				          " (= '%s'): %s"),
				        pkg_name(pkg, pnaw_nonambig), in,
				        result->buf, strerror(errno));
			debug(dbg_conffdetail, "conffderef nonexistent");
			return 0;
		} else if (S_ISREG(stab.st_mode)) {
			debug(dbg_conff, "conffderef in='%s' result='%s'",
			      in, result->buf);
			return 0;
		} else if (S_ISLNK(stab.st_mode)) {
			ssize_t linksize;

			debug(dbg_conffdetail, "conffderef symlink loopprotect=%d",
			      loopprotect);
			if (loopprotect++ >= 25) {
				warning(_("%s: config file '%s' is a circular link\n"
				          " (= '%s')"),
				        pkg_name(pkg, pnaw_nonambig), in,
				        result->buf);
				return -1;
			}

			linksize = file_readlink(result->buf, &target, stab.st_size);
			if (linksize < 0) {
				warning(_("%s: unable to readlink conffile '%s'\n"
				          " (= '%s'): %s"),
				        pkg_name(pkg, pnaw_nonambig), in,
				        result->buf, strerror(errno));
				return -1;
			} else if (linksize != stab.st_size) {
				warning(_("symbolic link '%.250s' size has "
				          "changed from %jd to %zd"),
				        result->buf, (intmax_t)stab.st_size,
				        linksize);
				/* If the returned size is smaller, let's
				 * proceed, otherwise error out. */
				if (linksize > stab.st_size)
					return -1;
			}

			debug(dbg_conffdetail,
			      "conffderef readlink gave %zd, '%s'",
			      linksize, target.buf);

			if (target.buf[0] == '/') {
				varbuf_set_str(result, dpkg_fsys_get_dir());
				debug(dbg_conffdetail,
				      "conffderef readlink absolute");
			} else {
				ssize_t r;

				for (r = result->used - 1; r > 0 && result->buf[r] != '/'; r--)
					;
				if (r < 0) {
					warning(_("%s: conffile '%.250s' resolves to degenerate filename\n"
					          " ('%s' is a symlink to '%s')"),
					        pkg_name(pkg, pnaw_nonambig),
					        in, result->buf, target.buf);
					return -1;
				}
				if (result->buf[r] == '/')
					r++;
				varbuf_trunc(result, r);
				debug(dbg_conffdetail,
				      "conffderef readlink relative to '%s'",
				      varbuf_str(result));
			}
			varbuf_add_varbuf(result, &target);
		} else {
			warning(_("%s: conffile '%.250s' is not a plain file or symlink (= '%s')"),
			        pkg_name(pkg, pnaw_nonambig), in, result->buf);
			return -1;
		}
	}
}

/**
 * Generate a file contents MD5 hash.
 *
 * The caller is responsible for providing a buffer for the hash result
 * at least MD5HASHLEN + 1 characters long.
 *
 * @param[in] pkg The package to act on.
 * @param[out] hashbuf The buffer to store the generated hash.
 * @param[in] fn The filename.
 */
void
md5hash(struct pkginfo *pkg, char *hashbuf, const char *fn)
{
	struct dpkg_error err;
	static int fd;

	fd = open(fn, O_RDONLY);

	if (fd >= 0) {
		push_cleanup(cu_closefd, ehflag_bombout, 1, &fd);
		if (fd_md5(fd, hashbuf, -1, &err) < 0)
			ohshit(_("cannot compute MD5 digest for file '%s': %s"),
			       fn, err.str);
		pop_cleanup(ehflag_normaltidy); /* fd = open(cdr.buf) */
		close(fd);
	} else if (errno == ENOENT) {
		strcpy(hashbuf, NONEXISTENTFLAG);
	} else {
		warning(_("%s: unable to open %s to compute its digest: %s"),
		        pkg_name(pkg, pnaw_nonambig), fn, strerror(errno));
		strcpy(hashbuf, EMPTYHASHFLAG);
	}
}
