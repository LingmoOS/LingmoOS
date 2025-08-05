/*
 * dpkg - main program for package management
 * trigproc.c - trigger processing
 *
 * Copyright © 2007 Canonical Ltd
 * written by Ian Jackson <ijackson@chiark.greenend.org.uk>
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

#include <sys/stat.h>

#include <fcntl.h>
#include <stdlib.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg.h>
#include <dpkg/pkg-queue.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>
#include <dpkg/triglib.h>

#include "main.h"

/*
 * Trigger processing algorithms:
 *
 *
 * There is a separate queue (‘deferred trigproc list’) for triggers
 * ‘relevant’ to what we just did; when we find something triggered ‘now’
 * we add it to that queue (unless --no-triggers).
 *
 *
 * We want to prefer configuring packages where possible to doing trigger
 * processing, although it would be better to prefer trigger processing
 * to cycle-breaking we need to do the latter first or we might generate
 * artificial trigger cycles, but we always want to prefer trigger
 * processing to dependency forcing. This is achieved as follows:
 *
 * Each time during configure processing where a package D is blocked by
 * only (ie Depends unsatisfied but would be satisfied by) a t-awaiter W
 * we make a note of (one of) W's t-pending, T. (Only the last such T.)
 * (If --no-triggers and nonempty argument list and package isn't in
 * argument list then we don't do this.)
 *
 * Each time in process_queue() where we increment dependtry, we instead
 * see if we have encountered such a t-pending T. If we do and are in
 * a trigger processing try, we trigproc T instead of incrementing
 * dependtry and this counts as having done something so we reset
 * sincenothing.
 *
 *
 * For --triggers-only and --configure, we go through each thing in the
 * argument queue (the enqueue_package queue) and check what its state is
 * and if appropriate we trigproc it. If we didn't have a queue (or had
 * just --pending) we search all triggers-pending packages and add them
 * to the deferred trigproc list.
 *
 *
 * Before quitting from most operations, we trigproc each package in the
 * deferred trigproc list. This may (if not --no-triggers) of course add
 * new things to the deferred trigproc list.
 *
 *
 * Note that ‘we trigproc T’ must involve trigger cycle detection and
 * also automatic setting of t-awaiters to t-pending or installed. In
 * particular, we do cycle detection even for trigger processing in the
 * configure dependtry loop (and it is OK to do it for explicitly
 * specified packages from the command line arguments; duplicates are
 * removed by packages.c:process_queue).
 */

/*========== Deferred trigger queue. ==========*/

static struct pkg_queue deferred = PKG_QUEUE_INIT;

static void
trigproc_enqueue_deferred(struct pkginfo *pend)
{
	if (f_triggers < 0)
		return;
	ensure_package_clientdata(pend);
	if (pend->clientdata->trigprocdeferred)
		return;
	pend->clientdata->trigprocdeferred = pkg_queue_push(&deferred, pend);
	debug(dbg_triggers, "trigproc_enqueue_deferred pend=%s",
	      pkg_name(pend, pnaw_always));
}

/**
 * Populate the deferred trigger queue.
 *
 * When dpkg is called with a specific set of packages to act on, we might
 * have packages pending trigger processing. But because there are frontends
 * that do not perform a final «dpkg --configure --pending» call (i.e. apt),
 * the system is left in a state with packages not fully installed.
 *
 * We have to populate the deferred trigger queue from the entire package
 * database, so that we might try to do opportunistic trigger processing
 * when going through the deferred trigger queue, because a fixed apt will
 * not request the necessary processing anyway.
 *
 * XXX: This can be removed once apt is fixed in the next stable release.
 */
void
trigproc_populate_deferred(void)
{
	struct pkg_hash_iter *iter;
	struct pkginfo *pkg;

	iter = pkg_hash_iter_new();
	while ((pkg = pkg_hash_iter_next_pkg(iter))) {
		if (!pkg->trigpend_head)
			continue;

		if (pkg->status != PKG_STAT_TRIGGERSAWAITED &&
		    pkg->status != PKG_STAT_TRIGGERSPENDING)
			continue;

		if (pkg->want != PKG_WANT_INSTALL &&
		    pkg->want != PKG_WANT_HOLD)
			continue;

		trigproc_enqueue_deferred(pkg);
	}
	pkg_hash_iter_free(iter);
}

void
trigproc_run_deferred(void)
{
	jmp_buf ejbuf;

	debug(dbg_triggers, "trigproc_run_deferred");
	while (!pkg_queue_is_empty(&deferred)) {
		struct pkginfo *pkg;

		pkg  = pkg_queue_pop(&deferred);
		if (!pkg)
			continue;

		if (setjmp(ejbuf)) {
			pop_error_context(ehflag_bombout);
			continue;
		}
		push_error_context_jump(&ejbuf, print_error_perpackage,
		                        pkg_name(pkg, pnaw_nonambig));

		ensure_package_clientdata(pkg);
		pkg->clientdata->trigprocdeferred = NULL;
		trigproc(pkg, TRIGPROC_TRY_DEFERRED);

		pop_error_context(ehflag_normaltidy);
	}
}

/*
 * Called by modstatdb_note.
 */
void
trig_activate_packageprocessing(struct pkginfo *pkg)
{
	debug(dbg_triggersdetail, "trigproc_activate_packageprocessing pkg=%s",
	      pkg_name(pkg, pnaw_always));

	trig_parse_ci(pkg_infodb_get_file(pkg, &pkg->installed, TRIGGERSCIFILE),
	              NULL, trig_cicb_statuschange_activate,
	              pkg, &pkg->installed);
}

/*========== Actual trigger processing. ==========*/

struct trigcyclenode {
	struct trigcyclenode *next;
	struct trigcycleperpkg *pkgs;
	struct pkginfo *then_processed;
};

struct trigcycleperpkg {
	struct trigcycleperpkg *next;
	struct pkginfo *pkg;
	struct trigpend *then_trigs;
};

static bool tortoise_advance;
static struct trigcyclenode *tortoise, *hare;

void
trigproc_reset_cycle(void)
{
	tortoise_advance = false;
	tortoise = hare = NULL;
}

static bool
tortoise_in_hare(struct pkginfo *processing_now,
                 struct trigcycleperpkg *tortoise_pkg)
{
	const char *processing_now_name, *tortoise_name;
	struct trigpend *hare_trig, *tortoise_trig;

	processing_now_name = pkg_name(processing_now, pnaw_nonambig);
	tortoise_name = pkg_name(tortoise_pkg->pkg, pnaw_nonambig);

	debug(dbg_triggersdetail, "%s pnow=%s tortoise=%s", __func__,
	      processing_now_name, tortoise_name);
	for (tortoise_trig = tortoise_pkg->then_trigs;
	     tortoise_trig;
	     tortoise_trig = tortoise_trig->next) {
		debug(dbg_triggersdetail,
		      "%s pnow=%s tortoise=%s tortoisetrig=%s", __func__,
		      processing_now_name, tortoise_name, tortoise_trig->name);

		/* hare is now so we can just look up in the actual data. */
		for (hare_trig = tortoise_pkg->pkg->trigpend_head;
		     hare_trig;
		     hare_trig = hare_trig->next) {
			debug(dbg_triggersstupid, "%s pnow=%s tortoise=%s"
			      " tortoisetrig=%s haretrig=%s", __func__,
			      processing_now_name, tortoise_name,
			      tortoise_trig->name, hare_trig->name);
			if (strcmp(hare_trig->name, tortoise_trig->name) == 0)
				break;
		}

		if (hare_trig == NULL) {
			/* Not found in hare, yay! */
			debug(dbg_triggersdetail, "%s pnow=%s tortoise=%s OK",
			      __func__, processing_now_name, tortoise_name);
			return false;
		}
	}

	return true;
}

static struct trigcyclenode *
trigproc_new_cyclenode(struct pkginfo *processing_now)
{
	struct trigcyclenode *tcn;
	struct trigcycleperpkg *tcpp;
	struct pkginfo *pkg;
	struct pkg_hash_iter *iter;

	tcn = nfmalloc(sizeof(*tcn));
	tcn->pkgs = NULL;
	tcn->next = NULL;
	tcn->then_processed = processing_now;

	iter = pkg_hash_iter_new();
	while ((pkg = pkg_hash_iter_next_pkg(iter))) {
		if (!pkg->trigpend_head)
			continue;
		tcpp = nfmalloc(sizeof(*tcpp));
		tcpp->pkg = pkg;
		tcpp->then_trigs = pkg->trigpend_head;
		tcpp->next = tcn->pkgs;
		tcn->pkgs = tcpp;
	}
	pkg_hash_iter_free(iter);

	return tcn;
}

/*
 * Returns package we are to give up on.
 */
static struct pkginfo *
check_trigger_cycle(struct pkginfo *processing_now)
{
	struct trigcyclenode *tcn;
	struct trigcycleperpkg *tortoise_pkg;
	struct trigpend *tortoise_trig;
	struct pkginfo *giveup;
	const char *sep;

	debug(dbg_triggers, "check_triggers_cycle pnow=%s",
	      pkg_name(processing_now, pnaw_always));

	tcn = trigproc_new_cyclenode(processing_now);

	if (!hare) {
		debug(dbg_triggersdetail, "check_triggers_cycle pnow=%s first",
		      pkg_name(processing_now, pnaw_always));
		hare = tortoise = tcn;
		return NULL;
	}

	hare->next = tcn;
	hare = hare->next;
	if (tortoise_advance)
		tortoise = tortoise->next;
	tortoise_advance = !tortoise_advance;

	/* Now we compare hare to tortoise.
	 * We want to find a trigger pending in tortoise which is not in hare
	 * if we find such a thing we have proved that hare isn't a superset
	 * of tortoise and so that we haven't found a loop (yet). */
	for (tortoise_pkg = tortoise->pkgs;
	     tortoise_pkg;
	     tortoise_pkg = tortoise_pkg->next) {
		if (!tortoise_in_hare(processing_now, tortoise_pkg))
			return NULL;
	}
	/* Oh dear. hare is a superset of tortoise. We are making no
	 * progress. */
	notice(_("cycle found while processing triggers:\n"
	         " chain of packages whose triggers are or may be responsible:"));
	sep = "  ";
	for (tcn = tortoise; tcn; tcn = tcn->next) {
		fprintf(stderr, "%s%s", sep,
		        pkg_name(tcn->then_processed, pnaw_nonambig));
		sep = " -> ";
	}
	fprintf(stderr, _("\n" " packages' pending triggers which are"
	                  " or may be unresolvable:\n"));
	for (tortoise_pkg = tortoise->pkgs;
	     tortoise_pkg;
	     tortoise_pkg = tortoise_pkg->next) {
		fprintf(stderr, "  %s",
		        pkg_name(tortoise_pkg->pkg, pnaw_nonambig));
		sep = ": ";
		for (tortoise_trig = tortoise_pkg->then_trigs;
		     tortoise_trig;
		     tortoise_trig = tortoise_trig->next) {
			fprintf(stderr, "%s%s", sep, tortoise_trig->name);
		}
		fprintf(stderr, "\n");
	}

	/* We give up on the _earliest_ package involved. */
	giveup = tortoise->pkgs->pkg;
	debug(dbg_triggers, "check_triggers_cycle pnow=%s giveup=%s",
	      pkg_name(processing_now, pnaw_always),
	      pkg_name(giveup, pnaw_always));
	if (giveup->status != PKG_STAT_TRIGGERSAWAITED &&
	    giveup->status != PKG_STAT_TRIGGERSPENDING)
		internerr("package %s in non-trigger state %s",
		          pkg_name(giveup, pnaw_always),
		          pkg_status_name(giveup));
	giveup->clientdata->istobe = PKG_ISTOBE_NORMAL;
	pkg_set_status(giveup, PKG_STAT_HALFCONFIGURED);
	modstatdb_note(giveup);
	print_error_perpackage(_("triggers looping, abandoned"),
	                       pkg_name(giveup, pnaw_nonambig));

	return giveup;
}

/*
 * Does cycle checking. Doesn't mind if pkg has no triggers pending - in
 * that case does nothing but fix up any stale awaiters.
 */
void
trigproc(struct pkginfo *pkg, enum trigproc_type type)
{
	static struct varbuf namesarg;

	struct varbuf depwhynot = VARBUF_INIT;

	debug(dbg_triggers, "trigproc %s", pkg_name(pkg, pnaw_always));

	ensure_package_clientdata(pkg);
	if (pkg->clientdata->trigprocdeferred)
		pkg->clientdata->trigprocdeferred->pkg = NULL;
	pkg->clientdata->trigprocdeferred = NULL;

	if (pkg->trigpend_head) {
		struct pkginfo *gaveup;
		struct trigpend *tp;
		enum dep_check ok;

		if (pkg->status != PKG_STAT_TRIGGERSPENDING &&
		    pkg->status != PKG_STAT_TRIGGERSAWAITED)
			internerr("package %s in non-trigger state %s",
			          pkg_name(pkg, pnaw_always),
			          pkg_status_name(pkg));

		if (dependtry < DEPEND_TRY_TRIGGERS &&
		    type == TRIGPROC_TRY_QUEUED) {
			/* We are not yet in a triggers run, so postpone this
			 * package completely. */
			enqueue_package(pkg);
			return;
		}

		if (dependtry >= DEPEND_TRY_CYCLES) {
			if (findbreakcycle(pkg))
				sincenothing = 0;
		}

		ok = dependencies_ok(pkg, NULL, &depwhynot);
		if (ok == DEP_CHECK_DEFER) {
			if (dependtry >= DEPEND_TRY_TRIGGERS_CYCLES) {
				gaveup = check_trigger_cycle(pkg);
				if (gaveup == pkg)
					return;
			}

			varbuf_destroy(&depwhynot);
			enqueue_package(pkg);
			return;
		} else if (ok == DEP_CHECK_HALT) {
			/* When doing opportunistic deferred trigger processing,
			 * nothing requires us to be able to make progress;
			 * skip the package and silently ignore the error due
			 * to unsatisfiable dependencies. And because we can
			 * end up here repeatedly, if this package is required
			 * to make progress for other packages, we need to
			 * reset the trigger cycle tracking to avoid detecting
			 * bogus cycles*/
			if (type == TRIGPROC_TRY_DEFERRED) {
				trigproc_reset_cycle();

				varbuf_destroy(&depwhynot);
				return;
			}

			sincenothing = 0;
			notice(_("dependency problems prevent processing "
			         "triggers for %s:\n%s"),
			       pkg_name(pkg, pnaw_nonambig),
			       varbuf_str(&depwhynot));
			varbuf_destroy(&depwhynot);
			ohshit(_("dependency problems - leaving triggers unprocessed"));
		} else if (depwhynot.used) {
			notice(_("%s: dependency problems, but processing "
			         "triggers anyway as you requested:\n%s"),
			       pkg_name(pkg, pnaw_nonambig),
			       varbuf_str(&depwhynot));
			varbuf_destroy(&depwhynot);
		}

		gaveup = check_trigger_cycle(pkg);
		if (gaveup == pkg)
			return;

		printf(_("Processing triggers for %s (%s) ...\n"),
		       pkg_name(pkg, pnaw_nonambig),
		       versiondescribe(&pkg->installed.version, vdew_nonambig));
		log_action("trigproc", pkg, &pkg->installed);

		varbuf_reset(&namesarg);
		for (tp = pkg->trigpend_head; tp; tp = tp->next) {
			varbuf_add_char(&namesarg, ' ');
			varbuf_add_str(&namesarg, tp->name);
		}

		/* Setting the status to half-configured
		 * causes modstatdb_note to clear pending triggers. */
		pkg_set_status(pkg, PKG_STAT_HALFCONFIGURED);
		modstatdb_note(pkg);

		if (f_act) {
			sincenothing = 0;
			maintscript_postinst(pkg, "triggered",
			                     varbuf_str(&namesarg) + 1, NULL);
		}

		post_postinst_tasks(pkg, PKG_STAT_INSTALLED);
	} else {
		/* In other branch is done by modstatdb_note(), from inside
		 * post_postinst_tasks(). */
		trig_clear_awaiters(pkg);
	}
}

/*========== Transitional global activation. ==========*/

static void
transitional_interest_callback_ro(const char *trig, struct pkginfo *pkg,
                                  struct pkgbin *pkgbin, enum trig_options opts)
{
	struct pkginfo *pend = pkg;
	struct pkgbin *pendbin = pkgbin;

	debug(dbg_triggersdetail,
	      "trig_transitional_interest_callback trig=%s pend=%s",
	      trig, pkgbin_name(pend, pendbin, pnaw_always));
	if (pend->status >= PKG_STAT_TRIGGERSAWAITED)
		trig_note_pend(pend, nfstrsave(trig));
}

static void
transitional_interest_callback(const char *trig, struct pkginfo *pkg,
                               struct pkgbin *pkgbin, enum trig_options opts)
{
	struct pkginfo *pend = pkg;
	struct pkgbin *pendbin = pkgbin;

	trig_cicb_interest_add(trig, pend, pendbin, opts);
	transitional_interest_callback_ro(trig, pend, pendbin, opts);
}

/*
 * cstatus might be msdbrw_readonly if we're in --no-act mode, in which
 * case we don't write out all of the interest files etc. but we do
 * invent all of the activations for our own benefit.
 */
static void
trig_transitional_activate(enum modstatdb_rw cstatus)
{
	struct pkg_hash_iter *iter;
	struct pkginfo *pkg;

	iter = pkg_hash_iter_new();
	while ((pkg = pkg_hash_iter_next_pkg(iter))) {
		if (pkg->status <= PKG_STAT_HALFINSTALLED)
			continue;
		debug(dbg_triggersdetail, "trig_transitional_activate %s %s",
		      pkg_name(pkg, pnaw_always),
		      pkg_status_name(pkg));
		pkg->trigpend_head = NULL;
		trig_parse_ci(pkg_infodb_get_file(pkg, &pkg->installed,
		                                  TRIGGERSCIFILE),
		              cstatus >= msdbrw_write ?
		              transitional_interest_callback :
		              transitional_interest_callback_ro, NULL,
		              pkg, &pkg->installed);
		/* Ensure we're not creating incoherent data that can't
		 * be written down. This should never happen in theory but
		 * can happen if you restore an old status file that is
		 * not in sync with the infodb files. */
		if (pkg->status < PKG_STAT_TRIGGERSAWAITED)
			continue;

		if (pkg->trigaw.head)
			pkg_set_status(pkg, PKG_STAT_TRIGGERSAWAITED);
		else if (pkg->trigpend_head)
			pkg_set_status(pkg, PKG_STAT_TRIGGERSPENDING);
		else
			pkg_set_status(pkg, PKG_STAT_INSTALLED);
	}
	pkg_hash_iter_free(iter);

	if (cstatus >= msdbrw_write) {
		modstatdb_checkpoint();
		trig_file_interests_save();
	}
}

/*========== Hook setup. ==========*/

TRIGHOOKS_DEFINE_NAMENODE_ACCESSORS

static const struct trig_hooks trig_our_hooks = {
	.enqueue_deferred = trigproc_enqueue_deferred,
	.transitional_activate = trig_transitional_activate,
	.namenode_find = th_nn_find,
	.namenode_interested = th_nn_interested,
	.namenode_name = th_nn_name,
};

void
trigproc_install_hooks(void)
{
	trig_override_hooks(&trig_our_hooks);
}
