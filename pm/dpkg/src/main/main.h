/*
 * dpkg - main program for package management
 * main.h - external definitions for this program
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006, 2008-2016 Guillem Jover <guillem@debian.org>
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

#ifndef MAIN_H
#define MAIN_H

#include <stdbool.h>

#include <dpkg/debug.h>
#include <dpkg/pkg-list.h>

#include "force.h"
#include "actions.h"
#include "security-mac.h"

/* These two are defined in <dpkg/fsys.h>. */
struct fsys_namenode_list;
struct fsys_namenode;

enum pkg_istobe {
	/** Package is to be left in a normal state. */
	PKG_ISTOBE_NORMAL,
	/** Package is to be removed. */
	PKG_ISTOBE_REMOVE,
	/** Package is to be installed, configured or triggered. */
	PKG_ISTOBE_INSTALLNEW,
	/** Package is to be deconfigured. */
	PKG_ISTOBE_DECONFIGURE,
	/** Package is to be checked for Pre-Depends satisfiability. */
	PKG_ISTOBE_PREINSTALL,
};

enum pkg_cycle_color {
	PKG_CYCLE_WHITE,
	PKG_CYCLE_GRAY,
	PKG_CYCLE_BLACK,
};

struct perpackagestate {
  enum pkg_istobe istobe;

  /** Used during cycle detection. */
  enum pkg_cycle_color color;

  bool enqueued;

  int replacingfilesandsaid;
  int cmdline_seen;

  /** Non-NULL iff in trigproc.c:deferred. */
  struct pkg_list *trigprocdeferred;
};

extern const char *const statusstrings[];

extern int f_act;
extern int f_alsoselect;
extern int f_autodeconf;
extern int f_debsig;
extern int f_pending;
extern int f_recursive;
extern int f_robot;
extern int f_skipsame;
extern int f_triggers;

extern bool abort_processing;
extern int errabort;
extern struct pkg_list *ignoredependss;

struct invoke_hook {
	struct invoke_hook *next;
	char *command;
};

struct invoke_list {
	struct invoke_hook *head, **tail;
};

/* from perpkgstate.c */

void ensure_package_clientdata(struct pkginfo *pkg);

/* from archives.c */

int archivefiles(const char *const *argv);
void process_archive(const char *filename);
bool wanttoinstall(struct pkginfo *pkg);

/* from update.c */

int forgetold(const char *const *argv);
int updateavailable(const char *const *argv);

/* from enquiry.c */

extern const char *assert_feature_name;

int audit(const char *const *argv);
int unpackchk(const char *const *argv);
int assert_feature(const char *const *argv);
int validate_pkgname(const char *const *argv);
int validate_trigname(const char *const *argv);
int validate_archname(const char *const *argv);
int validate_version(const char *const *argv);
int predeppackage(const char *const *argv);
int printarch(const char *const *argv);
int printinstarch(const char *const *argv);
int print_foreign_arches(const char *const *argv);
int cmpversions(const char *const *argv);

/* from verify.c */

bool verify_set_output(const char *name);
int verify(const char *const *argv);

/* from select.c */

int getselections(const char *const *argv);
int setselections(const char *const *argv);
int clearselections(const char *const *argv);

/* from packages.c, remove.c and configure.c */

void md5hash(struct pkginfo *pkg, char *hashbuf, const char *fn);
void enqueue_package(struct pkginfo *pkg);
void enqueue_package_mark_seen(struct pkginfo *pkg);
void process_queue(void);
int packages(const char *const *argv);
void removal_bulk(struct pkginfo *pkg);
int conffderef(struct pkginfo *pkg, struct varbuf *result, const char *in);

enum dep_check {
  DEP_CHECK_HALT = 0,
  DEP_CHECK_DEFER = 1,
  DEP_CHECK_OK = 2,
};

enum dep_check dependencies_ok(struct pkginfo *pkg, struct pkginfo *removing,
                               struct varbuf *aemsgs);
enum dep_check breakses_ok(struct pkginfo *pkg, struct varbuf *aemsgs);

void deferred_remove(struct pkginfo *pkg);
void deferred_configure(struct pkginfo *pkg);

/*
 * During the packages queue processing, the algorithm for deciding what to
 * configure first is as follows:
 *
 * Loop through all packages doing a ‘try 1’ until we've been round and
 * nothing has been done, then do ‘try 2’, and subsequent ones likewise.
 * The incrementing of ‘dependtry’ is done by process_queue().
 *
 * Try 1:
 *   Are all dependencies of this package done? If so, do it.
 *   Are any of the dependencies missing or the wrong version?
 *     If so, abort (unless --force-depends, in which case defer).
 *   Will we need to configure a package we weren't given as an
 *     argument? If so, abort ─ except if --force-configure-any,
 *     in which case we add the package to the argument list.
 *   If none of the above, defer the package.
 *
 * Try 2:
 *   Find a cycle and break it (see above).
 *   Do as for try 1.
 *
 * Try 3:
 *   Start processing triggers if necessary.
 *   Do as for try 2.
 *
 * Try 4:
 *   Same as for try 3, but check trigger cycles even when deferring
 *   processing due to unsatisfiable dependencies.
 *
 * Try 5 (only if --force-depends-version):
 *   Same as for try 2, but don't mind version number in dependencies.
 *
 * Try 6 (only if --force-depends):
 *   Do anyway.
 */
enum dependtry {
	DEPEND_TRY_NORMAL = 1,
	DEPEND_TRY_CYCLES = 2,
	DEPEND_TRY_TRIGGERS = 3,
	DEPEND_TRY_TRIGGERS_CYCLES = 4,
	DEPEND_TRY_FORCE_DEPENDS_VERSION = 5,
	DEPEND_TRY_FORCE_DEPENDS = 6,
	DEPEND_TRY_LAST,
};

extern enum dependtry dependtry;
extern int sincenothing;

/* from cleanup.c (most of these are declared in archives.h) */

void cu_prermremove(int argc, void **argv);

/* from errors.c */

void print_error_perpackage(const char *emsg, const void *data);
void print_error_perarchive(const char *emsg, const void *data);
int reportbroken_retexitstatus(int ret);
bool skip_due_to_hold(struct pkginfo *pkg);

/* from help.c */

struct stat;

bool ignore_depends(const struct pkginfo *pkg);
bool force_breaks(struct deppossi *possi);
bool force_depends(struct deppossi *possi);
bool force_conflicts(struct deppossi *possi);
bool
conffile_is_disappearing(struct conffile *conff);
void
conffile_mark_obsolete(struct pkginfo *pkg, struct fsys_namenode *namenode);
void pkg_conffiles_mark_old(struct pkginfo *pkg);
void checkpath(void);

struct fsys_namenode *
namenodetouse(struct fsys_namenode *namenode,
              struct pkginfo *pkg, struct pkgbin *pkgbin);

int maintscript_installed(struct pkginfo *pkg, const char *scriptname,
                          const char *desc, ...) DPKG_ATTR_SENTINEL;
int maintscript_new(struct pkginfo *pkg,
                    const char *scriptname, const char *desc,
                    const char *cidir, char *cidirrest, ...)
	DPKG_ATTR_SENTINEL;
int maintscript_fallback(struct pkginfo *pkg,
                         const char *scriptname, const char *desc,
                         const char *cidir, char *cidirrest,
                         const char *ifok, const char *iffallback);

/* Callers wanting to run the postinst use these two as they want to postpone
 * trigger incorporation until after updating the package status. The effect
 * is that a package can trigger itself. */
int maintscript_postinst(struct pkginfo *pkg, ...) DPKG_ATTR_SENTINEL;
void post_postinst_tasks(struct pkginfo *pkg, enum pkgstatus new_status);

void clear_istobes(void);
bool
dir_is_used_by_others(struct fsys_namenode *namenode, struct pkginfo *pkg);
bool
dir_is_used_by_pkg(struct fsys_namenode *namenode, struct pkginfo *pkg,
                   struct fsys_namenode_list *list);
bool
dir_has_conffiles(struct fsys_namenode *namenode, struct pkginfo *pkg);

void log_action(const char *action, struct pkginfo *pkg, struct pkgbin *pkgbin);

/* from trigproc.c */

enum trigproc_type {
	/** Opportunistic deferred trigger processing. */
	TRIGPROC_TRY_DEFERRED,
	/** Opportunistic queued trigger processing. */
	TRIGPROC_TRY_QUEUED,
	/** Required trigger processing. */
	TRIGPROC_REQUIRED,
};

void trigproc_install_hooks(void);
void trigproc_populate_deferred(void);
void trigproc_run_deferred(void);
void trigproc_reset_cycle(void);

void trigproc(struct pkginfo *pkg, enum trigproc_type type);

void trig_activate_packageprocessing(struct pkginfo *pkg);

/* from depcon.c */

enum which_pkgbin {
  wpb_installed,
  wpb_available,
  wpb_by_istobe,
};

struct deppossi_pkg_iterator;

struct deppossi_pkg_iterator *
deppossi_pkg_iter_new(struct deppossi *possi, enum which_pkgbin wpb);
struct pkginfo *
deppossi_pkg_iter_next(struct deppossi_pkg_iterator *iter);
void
deppossi_pkg_iter_free(struct deppossi_pkg_iterator *iter);

bool depisok(struct dependency *dep, struct varbuf *whynot,
             struct pkginfo **fixbyrm, struct pkginfo **fixbytrigaw,
             bool allowunconfigd);
struct cyclesofarlink;
bool findbreakcycle(struct pkginfo *pkg);
void describedepcon(struct varbuf *addto, struct dependency *dep);

#endif /* MAIN_H */
