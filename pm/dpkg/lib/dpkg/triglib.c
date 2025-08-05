/*
 * libdpkg - Debian packaging suite library routines
 * triglib.c - trigger handling
 *
 * Copyright © 2007 Canonical Ltd
 * Written by Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2015 Guillem Jover <guillem@debian.org>
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
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg.h>
#include <dpkg/dlist.h>
#include <dpkg/dir.h>
#include <dpkg/pkg-spec.h>
#include <dpkg/trigdeferred.h>
#include <dpkg/triglib.h>

/*========== Recording triggers. ==========*/

static char *triggersdir, *triggersfilefile;

static char *
trig_get_filename(const char *dir, const char *filename)
{
	return str_fmt("%s/%s", dir, filename);
}

static struct trig_hooks trigh;

/*---------- Noting trigger activation in memory. ----------*/

/*
 * Called via trig_*activate* et al from:
 *   - trig_incorporate: reading of Unincorp (explicit trigger activations)
 *   - various places: processing start (‘activate’ in triggers ci file)
 *   - namenodetouse: file triggers during unpack / remove
 *   - deferred_configure: file triggers during config file processing
 *
 * Not called from trig_transitional_activation; that runs
 * trig_note_pend directly which means that (a) awaiters are not
 * recorded (how would we know?) and (b) we don't enqueue them for
 * deferred processing in this run.
 *
 * We add the trigger to Triggers-Pending first. This makes it
 * harder to get into the state where Triggers-Awaited for aw lists
 * pend but Triggers-Pending for pend is empty. (See also the
 * comment in deppossi_ok_found regarding this situation.)
 */

/*
 * aw might be NULL.
 * trig is not copied!
 */
static void
trig_record_activation(struct pkginfo *pend, struct pkginfo *aw, const char *trig)
{
	if (pend->status < PKG_STAT_TRIGGERSAWAITED)
		return; /* Not interested then. */

	if (trig_note_pend(pend, trig))
		modstatdb_note_ifwrite(pend);

	if (trigh.enqueue_deferred)
		trigh.enqueue_deferred(pend);

	if (aw && pend->status > PKG_STAT_CONFIGFILES)
		if (trig_note_aw(pend, aw)) {
			if (aw->status > PKG_STAT_TRIGGERSAWAITED)
				pkg_set_status(aw, PKG_STAT_TRIGGERSAWAITED);
			modstatdb_note_ifwrite(aw);
		}
}

void
trig_clear_awaiters(struct pkginfo *notpend)
{
	struct trigaw *ta;

	if (notpend->trigpend_head)
		internerr("package %s has pending triggers",
		          pkg_name(notpend, pnaw_always));

	ta = notpend->othertrigaw_head;
	notpend->othertrigaw_head = NULL;
	for (; ta; ta = ta->samepend_next) {
		struct pkginfo *aw;

		aw = ta->aw;
		if (!aw)
			continue;
		LIST_UNLINK_PART(aw->trigaw, ta, sameaw);
		if (!aw->trigaw.head && aw->status == PKG_STAT_TRIGGERSAWAITED) {
			if (aw->trigpend_head)
				pkg_set_status(aw, PKG_STAT_TRIGGERSPENDING);
			else
				pkg_set_status(aw, PKG_STAT_INSTALLED);
			modstatdb_note(aw);
		}
	}
}

/*
 * Fix up packages in state triggers-awaited w/o the corresponding package
 * with pending triggers. This can happen when dpkg was interrupted
 * while in modstatdb_note, and the package in triggers-pending had its
 * state modified but dpkg could not finish clearing the awaiters.
 *
 * XXX: Possibly get rid of some of the checks done somewhere else for
 *      this condition at run-time.
 */
void
trig_fixup_awaiters(enum modstatdb_rw cstatus)
{
	if (cstatus < msdbrw_write)
		return;

	trig_awaited_pend_foreach(trig_clear_awaiters);
	trig_awaited_pend_free();
}

/*---------- Generalized handling of trigger kinds. ----------*/

struct trigkindinfo {
	/* Only for trig_activate_start. */
	void (*activate_start)(void);

	/* Rest are for everyone: */
	void (*activate_awaiter)(struct pkginfo *pkg /* may be NULL */);
	void (*activate_done)(void);
	void (*interest_change)(const char *name, struct pkginfo *pkg,
	                        struct pkgbin *pkgbin,
	                        int signum, enum trig_options opts);
};

static const struct trigkindinfo tki_explicit, tki_file, tki_unknown;
static const struct trigkindinfo *dtki;

/* As passed into activate_start. */
static char *trig_activating_name;

static const struct trigkindinfo *
trig_classify_byname(const char *name)
{
	if (name[0] == '/') {
		const char *slash;

		slash = name;
		while (slash) {
			if (slash[1] == '\0' || slash[1] == '/')
				goto invalid;

			slash = strchr(slash + 2, '/');
		}
		return &tki_file;
	}

	if (!pkg_name_is_illegal(name) && !strchr(name, '_'))
		return &tki_explicit;

invalid:
	return &tki_unknown;
}

static const char *
trig_dump_trigger_options(enum trig_options opts)
{
	if (opts == TRIG_NOAWAIT)
		return "/noawait";
	return "";
}

/*
 * Modifies the trigger string by removing the options at the ‘/’ delimiter
 * and truncating it with a NUL character.
 */
static enum trig_options
trig_parse_trigger_options(char *trigger)
{
	char *slash;

	slash = strchr(trigger, '/');
	if (slash == NULL)
		return TRIG_AWAIT;

	*slash++ = '\0';
	if (strcmp("noawait", slash) == 0)
		return TRIG_NOAWAIT;
	if (strcmp("await", slash) == 0)
		return TRIG_AWAIT;

	/* XXX: Should perhaps warn or error for unknown keywords. */

	return TRIG_AWAIT;
}

/*
 * Calling sequence is:
 *  trig_activate_start(triggername);
 *  dtki->activate_awaiter(awaiting_package); } zero or more times
 *  dtki->activate_awaiter(NULL);             }  in any order
 *  dtki->activate_done();
 */
static void
trig_activate_start(const char *name)
{
	dtki = trig_classify_byname(name);
	trig_activating_name = nfstrsave(name);
	dtki->activate_start();
}

/*---------- Unknown trigger kinds. ----------*/

static void
trk_unknown_activate_start(void)
{
}

static void
trk_unknown_activate_awaiter(struct pkginfo *aw)
{
}

static void
trk_unknown_activate_done(void)
{
}

static void DPKG_ATTR_NORET
trk_unknown_interest_change(const char *trig, struct pkginfo *pkg,
                            struct pkgbin *pkgbin, int signum,
                            enum trig_options opts)
{
	ohshit(_("invalid or unknown syntax in trigger name '%.250s'"
	         " (in trigger interests for package '%.250s')"),
	       trig, pkgbin_name(pkg, pkgbin, pnaw_nonambig));
}

static const struct trigkindinfo tki_unknown = {
	.activate_start = trk_unknown_activate_start,
	.activate_awaiter = trk_unknown_activate_awaiter,
	.activate_done = trk_unknown_activate_done,
	.interest_change = trk_unknown_interest_change,
};

/*---------- Explicit triggers. ----------*/

static FILE *trk_explicit_f;
static struct varbuf trk_explicit_fn;
static char *trk_explicit_trig;

static void
trk_explicit_activate_done(void)
{
	if (trk_explicit_f) {
		fclose(trk_explicit_f);
		trk_explicit_f = NULL;
	}
}

static void
trk_explicit_start(const char *trig)
{
	trk_explicit_activate_done();

	varbuf_reset(&trk_explicit_fn);
	varbuf_add_dir(&trk_explicit_fn, triggersdir);
	varbuf_add_str(&trk_explicit_fn, trig);

	trk_explicit_f = fopen(trk_explicit_fn.buf, "r");
	if (!trk_explicit_f) {
		if (errno != ENOENT)
			ohshite(_("failed to open trigger interest list file '%.250s'"),
			        trk_explicit_fn.buf);
	}
}

static int
trk_explicit_fgets(char *buf, size_t sz)
{
	return fgets_checked(buf, sz, trk_explicit_f, trk_explicit_fn.buf);
}

static void
trk_explicit_activate_start(void)
{
	trk_explicit_start(trig_activating_name);
	trk_explicit_trig = trig_activating_name;
}

static void
trk_explicit_activate_awaiter(struct pkginfo *aw)
{
	char buf[1024];

	if (!trk_explicit_f)
		return;

	if (fseek(trk_explicit_f, 0, SEEK_SET))
		ohshite(_("failed to rewind trigger interest file '%.250s'"),
		        trk_explicit_fn.buf);

	while (trk_explicit_fgets(buf, sizeof(buf)) >= 0) {
		struct pkginfo *pend;
		struct dpkg_error err;
		enum trig_options opts;

		opts = trig_parse_trigger_options(buf);

		pend = pkg_spec_parse_pkg(buf, &err);
		if (pend == NULL)
			ohshit(_("trigger interest file '%.250s' syntax error; "
			         "illegal package name '%.250s': %.250s"),
			       trk_explicit_fn.buf, buf, err.str);

		trig_record_activation(pend, opts == TRIG_NOAWAIT ? NULL : aw,
		                       trk_explicit_trig);
	}
}

static void
trk_explicit_interest_change(const char *trig,  struct pkginfo *pkg,
                             struct pkgbin *pkgbin, int signum,
                             enum trig_options opts)
{
	char buf[1024];
	struct atomic_file *file;
	bool empty = true;

	trk_explicit_start(trig);
	file = atomic_file_new(trk_explicit_fn.buf, 0);
	atomic_file_open(file);

	while (trk_explicit_f && trk_explicit_fgets(buf, sizeof(buf)) >= 0) {
		enum trig_options trig_opts;
		struct pkginfo *pkg_parsed;
		struct dpkg_error err;

		trig_opts = trig_parse_trigger_options(buf);

		pkg_parsed = pkg_spec_parse_pkg(buf, &err);
		if (pkg_parsed == NULL)
			ohshit(_("trigger interest file '%.250s' syntax error; "
			         "illegal package name '%.250s': %.250s"),
			       trk_explicit_fn.buf, buf, err.str);

		if (pkg == pkg_parsed &&
		    (pkgbin == &pkg_parsed->installed ||
		     pkgbin == &pkg_parsed->available))
			continue;

		fprintf(file->fp, "%s%s\n", buf,
		        trig_dump_trigger_options(trig_opts));
		empty = false;
	}
	if (signum > 0) {
		fprintf(file->fp, "%s%s\n",
		        pkgbin_name(pkg, pkgbin, pnaw_same),
		        trig_dump_trigger_options(opts));
		empty = false;
	}

	if (!empty)
		atomic_file_sync(file);

	atomic_file_close(file);

	if (empty)
		atomic_file_remove(file);
	else
		atomic_file_commit(file);

	atomic_file_free(file);

	dir_sync_path(triggersdir);
}

static const struct trigkindinfo tki_explicit = {
	.activate_start = trk_explicit_activate_start,
	.activate_awaiter = trk_explicit_activate_awaiter,
	.activate_done = trk_explicit_activate_done,
	.interest_change = trk_explicit_interest_change,
};

/*---------- File triggers. ----------*/

static struct {
	struct trigfileint *head, *tail;
} filetriggers;

/*
 * Values:
 *  -1: Not read.
 *   0: Not edited.
 *   1: Edited
 */
static int filetriggers_edited = -1;

/*
 * Called by various people with signum -1 and +1 to mean remove and add
 * and also by trig_file_interests_ensure() with signum +2 meaning add
 * but die if already present.
 */
static void
trk_file_interest_change(const char *trig, struct pkginfo *pkg,
                         struct pkgbin *pkgbin, int signum,
                         enum trig_options opts)
{
	struct fsys_namenode *fnn;
	struct trigfileint **search, *tfi;

	fnn = trigh.namenode_find(trig, signum <= 0);
	if (!fnn) {
		if (signum >= 0)
			internerr("lost filename node '%s' for package %s "
			          "triggered to add", trig,
			          pkgbin_name(pkg, pkgbin, pnaw_always));
		return;
	}

	for (search = trigh.namenode_interested(fnn);
	     (tfi = *search);
	     search = &tfi->samefile_next)
		if (tfi->pkg == pkg)
			goto found;

	/* Not found. */
	if (signum < 0)
		return;

	tfi = nfmalloc(sizeof(*tfi));
	tfi->pkg = pkg;
	tfi->pkgbin = pkgbin;
	tfi->fnn = fnn;
	tfi->options = opts;
	tfi->samefile_next = *trigh.namenode_interested(fnn);
	*trigh.namenode_interested(fnn) = tfi;

	LIST_LINK_TAIL_PART(filetriggers, tfi, inoverall);
	goto edited;

found:
	tfi->options = opts;
	if (signum > 1)
		ohshit(_("duplicate file trigger interest for filename '%.250s' "
		         "and package '%.250s'"), trig,
		       pkgbin_name(pkg, pkgbin, pnaw_nonambig));
	if (signum > 0)
		return;

	/* Remove it: */
	*search = tfi->samefile_next;
	LIST_UNLINK_PART(filetriggers, tfi, inoverall);
edited:
	filetriggers_edited = 1;
}

static void
trig_file_interests_remove(void)
{
	if (unlink(triggersfilefile) && errno != ENOENT)
		ohshite(_("cannot remove '%.250s'"), triggersfilefile);
}

static void
trig_file_interests_update(void)
{
	struct trigfileint *tfi;
	struct atomic_file *file;

	file = atomic_file_new(triggersfilefile, 0);
	atomic_file_open(file);

	for (tfi = filetriggers.head; tfi; tfi = tfi->inoverall.next)
		fprintf(file->fp, "%s %s%s\n", trigh.namenode_name(tfi->fnn),
		        pkgbin_name(tfi->pkg, tfi->pkgbin, pnaw_same),
		        trig_dump_trigger_options(tfi->options));

	atomic_file_sync(file);
	atomic_file_close(file);
	atomic_file_commit(file);
	atomic_file_free(file);
}

void
trig_file_interests_save(void)
{
	if (filetriggers_edited <= 0)
		return;

	if (!filetriggers.head)
		trig_file_interests_remove();
	else
		trig_file_interests_update();

	dir_sync_path(triggersdir);

	filetriggers_edited = 0;
}

void
trig_file_interests_ensure(void)
{
	FILE *f;
	char linebuf[1024], *space;
	struct pkginfo *pkg;
	struct pkgbin *pkgbin;

	if (filetriggers_edited >= 0)
		return;

	f = fopen(triggersfilefile, "r");
	if (!f) {
		if (errno == ENOENT)
			goto ok;
		ohshite(_("unable to read file triggers file '%.250s'"),
		        triggersfilefile);
	}

	push_cleanup(cu_closestream, ~0, 1, f);
	while (fgets_checked(linebuf, sizeof(linebuf), f, triggersfilefile) >= 0) {
		struct dpkg_error err;
		enum trig_options trig_opts;

		space = strchr(linebuf, ' ');
		if (!space || linebuf[0] != '/')
			ohshit(_("syntax error in file triggers file '%.250s'"),
			       triggersfilefile);
		*space++ = '\0';

		trig_opts = trig_parse_trigger_options(space);

		pkg = pkg_spec_parse_pkg(space, &err);
		if (pkg == NULL)
			ohshit(_("file triggers record mentions illegal "
			         "package name '%.250s' (for interest in file "
			         "'%.250s'): %.250s"), space, linebuf, err.str);
		pkgbin = &pkg->installed;

		trk_file_interest_change(linebuf, pkg, pkgbin, +2, trig_opts);
	}
	pop_cleanup(ehflag_normaltidy);
ok:
	filetriggers_edited = 0;
}

void
trig_file_activate_byname(const char *trig, struct pkginfo *aw)
{
	struct fsys_namenode *fnn = trigh.namenode_find(trig, 1);

	if (fnn)
		trig_file_activate(fnn, aw);
}

void
trig_file_activate(struct fsys_namenode *trig, struct pkginfo *aw)
{
	struct trigfileint *tfi;

	for (tfi = *trigh.namenode_interested(trig); tfi;
	     tfi = tfi->samefile_next)
		trig_record_activation(tfi->pkg, (tfi->options == TRIG_NOAWAIT) ?
		                       NULL : aw, trigh.namenode_name(trig));
}

static void
trig_file_activate_parents(const char *trig, struct pkginfo *aw)
{
	char *path, *slash;

	/* Traverse the whole pathname to activate all of its components. */
	path = m_strdup(trig);

	while ((slash = strrchr(path, '/'))) {
		*slash = '\0';
		trig_file_activate_byname(path, aw);
	}

	free(path);
}

void
trig_path_activate(struct fsys_namenode *trig, struct pkginfo *aw)
{
	trig_file_activate(trig, aw);
	trig_file_activate_parents(trigh.namenode_name(trig), aw);
}

static void
trig_path_activate_byname(const char *trig, struct pkginfo *aw)
{
	struct fsys_namenode *fnn = trigh.namenode_find(trig, 1);

	if (fnn)
		trig_file_activate(fnn, aw);

	trig_file_activate_parents(trig, aw);
}

static const char *trk_file_trig;

static void
trk_file_activate_start(void)
{
	trk_file_trig = trig_activating_name;
}

static void
trk_file_activate_awaiter(struct pkginfo *aw)
{
	trig_path_activate_byname(trk_file_trig, aw);
}

static void
trk_file_activate_done(void)
{
}

static const struct trigkindinfo tki_file = {
	.activate_start = trk_file_activate_start,
	.activate_awaiter = trk_file_activate_awaiter,
	.activate_done = trk_file_activate_done,
	.interest_change = trk_file_interest_change,
};

/*---------- Trigger control info file. ----------*/

static void
trig_cicb_interest_change(const char *trig, struct pkginfo *pkg,
                          struct pkgbin *pkgbin, int signum,
                          enum trig_options opts)
{
	const struct trigkindinfo *tki = trig_classify_byname(trig);

	if (filetriggers_edited < 0)
		internerr("trigger control file for package %s not read",
		          pkgbin_name(pkg, pkgbin, pnaw_always));

	tki->interest_change(trig, pkg, pkgbin, signum, opts);
}

void
trig_cicb_interest_delete(const char *trig, struct pkginfo *pkg,
                          struct pkgbin *pkgbin, enum trig_options opts)
{
	trig_cicb_interest_change(trig, pkg, pkgbin, -1, opts);
}

void
trig_cicb_interest_add(const char *trig, struct pkginfo *pkg,
                       struct pkgbin *pkgbin, enum trig_options opts)
{
	trig_cicb_interest_change(trig, pkg, pkgbin, +1, opts);
}

void
trig_cicb_statuschange_activate(const char *trig, struct pkginfo *pkg,
                                struct pkgbin *pkgbin, enum trig_options opts)
{
	struct pkginfo *aw = pkg;

	trig_activate_start(trig);
	dtki->activate_awaiter((opts == TRIG_NOAWAIT) ? NULL : aw);
	dtki->activate_done();
}

static void
parse_ci_call(const char *file, const char *cmd, trig_parse_cicb *cb,
              const char *trig, struct pkginfo *pkg, struct pkgbin *pkgbin,
              enum trig_options opts)
{
	const char *emsg;

	emsg = trig_name_is_illegal(trig);
	if (emsg)
		ohshit(_("triggers ci file '%.250s' contains illegal trigger "
		         "syntax in trigger name '%.250s': %.250s"),
		       file, trig, emsg);
	if (cb)
		cb(trig, pkg, pkgbin, opts);
}

void
trig_parse_ci(const char *file, trig_parse_cicb *interest,
              trig_parse_cicb *activate, struct pkginfo *pkg,
              struct pkgbin *pkgbin)
{
	FILE *f;
	char linebuf[MAXTRIGDIRECTIVE], *cmd, *spc, *eol;
	int l;

	f = fopen(file, "r");
	if (!f) {
		if (errno == ENOENT)
			return; /* No file is just like an empty one. */
		ohshite(_("unable to open triggers ci file '%.250s'"), file);
	}
	push_cleanup(cu_closestream, ~0, 1, f);

	while ((l = fgets_checked(linebuf, sizeof(linebuf), f, file)) >= 0) {
		for (cmd = linebuf; c_iswhite(*cmd); cmd++) ;
		if (*cmd == '#')
			continue;
		for (eol = linebuf + l; eol > cmd && c_iswhite(eol[-1]); eol--) ;
		if (eol == cmd)
			continue;
		*eol = '\0';

		for (spc = cmd; *spc && !c_iswhite(*spc); spc++) ;
		if (!*spc)
			ohshit(_("triggers ci file contains unknown directive syntax"));
		*spc++ = '\0';
		while (c_iswhite(*spc))
			spc++;
		if (strcmp(cmd, "interest") == 0 ||
		    strcmp(cmd, "interest-await") == 0) {
			parse_ci_call(file, cmd, interest, spc, pkg, pkgbin, TRIG_AWAIT);
		} else if (strcmp(cmd, "interest-noawait") == 0) {
			parse_ci_call(file, cmd, interest, spc, pkg, pkgbin, TRIG_NOAWAIT);
		} else if (strcmp(cmd, "activate") == 0 ||
		           strcmp(cmd, "activate-await") == 0) {
			parse_ci_call(file, cmd, activate, spc, pkg, pkgbin, TRIG_AWAIT);
		} else if (strcmp(cmd, "activate-noawait") == 0) {
			parse_ci_call(file, cmd, activate, spc, pkg, pkgbin, TRIG_NOAWAIT);
		} else {
			ohshit(_("triggers ci file contains unknown directive '%.250s'"),
			       cmd);
		}
	}
	pop_cleanup(ehflag_normaltidy); /* fclose() */
}

/*---------- Unincorp file incorporation. ----------*/

static void
tdm_incorp_trig_begin(const char *trig)
{
	trig_activate_start(trig);
}

static void
tdm_incorp_package(const char *awname)
{
	struct pkginfo *aw;

	if (strcmp(awname, "-") == 0)
		aw = NULL;
	else
		aw = pkg_spec_parse_pkg(awname, NULL);

	dtki->activate_awaiter(aw);
}

static void
tdm_incorp_trig_end(void)
{
	dtki->activate_done();
}

static const struct trigdefmeths tdm_incorp = {
	.trig_begin = tdm_incorp_trig_begin,
	.package = tdm_incorp_package,
	.trig_end = tdm_incorp_trig_end
};

void
trig_incorporate(enum modstatdb_rw cstatus)
{
	enum trigdef_update_status ur;
	enum trigdef_update_flags tduf;

	free(triggersdir);
	triggersdir = dpkg_db_get_path(TRIGGERSDIR);

	free(triggersfilefile);
	triggersfilefile = trig_get_filename(triggersdir, TRIGGERSFILEFILE);

	trigdef_set_methods(&tdm_incorp);
	trig_file_interests_ensure();

	tduf = TDUF_NO_LOCK_OK;
	if (cstatus >= msdbrw_write) {
		tduf |= TDUF_WRITE;
		if (trigh.transitional_activate)
			tduf |= TDUF_WRITE_IF_ENOENT;
	}

	ur = trigdef_update_start(tduf);
	if (ur == TDUS_ERROR_NO_DIR && cstatus >= msdbrw_write) {
		if (mkdir(triggersdir, 0755)) {
			if (errno != EEXIST)
				ohshite(_("unable to create triggers state"
				          " directory '%.250s'"), triggersdir);
		}
		ur = trigdef_update_start(tduf);
	}
	switch (ur) {
	case TDUS_ERROR_EMPTY_DEFERRED:
		return;
	case TDUS_ERROR_NO_DIR:
	case TDUS_ERROR_NO_DEFERRED:
		if (!trigh.transitional_activate)
			return;
	/* Fall through. */
	case TDUS_NO_DEFERRED:
		trigh.transitional_activate(cstatus);
		break;
	case TDUS_OK:
		/* Read and incorporate triggers. */
		trigdef_parse();
		break;
	default:
		internerr("unknown trigdef_update_start return value '%d'", ur);
	}

	/* Right, that's it. New (empty) Unincorp can be installed. */
	trigdef_process_done();
}

/*---------- Default hooks. ----------*/

TRIGHOOKS_DEFINE_NAMENODE_ACCESSORS

static struct trig_hooks trigh = {
	.enqueue_deferred = NULL,
	.transitional_activate = NULL,
	.namenode_find = th_nn_find,
	.namenode_interested = th_nn_interested,
	.namenode_name = th_nn_name,
};

void
trig_override_hooks(const struct trig_hooks *hooks)
{
	trigh = *hooks;
}
