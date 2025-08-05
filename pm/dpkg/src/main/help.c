/*
 * dpkg - main program for package management
 * help.c - various helper routines
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2007-2015 Guillem Jover <guillem@debian.org>
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
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/path.h>
#include <dpkg/file.h>
#include <dpkg/command.h>
#include <dpkg/db-fsys.h>

#include "main.h"

const char *const statusstrings[]= {
  [PKG_STAT_NOTINSTALLED]    = N_("not installed"),
  [PKG_STAT_CONFIGFILES]     = N_("not installed but configs remain"),
  [PKG_STAT_HALFINSTALLED]   = N_("broken due to failed removal or installation"),
  [PKG_STAT_UNPACKED]        = N_("unpacked but not configured"),
  [PKG_STAT_HALFCONFIGURED]  = N_("broken due to postinst failure"),
  [PKG_STAT_TRIGGERSAWAITED] = N_("awaiting trigger processing by another package"),
  [PKG_STAT_TRIGGERSPENDING] = N_("triggered"),
  [PKG_STAT_INSTALLED]       = N_("installed")
};

struct fsys_namenode *
namenodetouse(struct fsys_namenode *namenode, struct pkginfo *pkg,
              struct pkgbin *pkgbin)
{
  struct fsys_namenode *fnn;

  if (!namenode->divert)
    return namenode;

  debug(dbg_eachfile, "namenodetouse namenode='%s' pkg=%s",
        namenode->name, pkgbin_name(pkg, pkgbin, pnaw_always));

  fnn = (namenode->divert->useinstead && namenode->divert->pkgset != pkg->set)
        ? namenode->divert->useinstead : namenode;

  debug(dbg_eachfile,
        "namenodetouse ... useinstead=%s camefrom=%s pkg=%s return %s",
        namenode->divert->useinstead ? namenode->divert->useinstead->name : "<none>",
        namenode->divert->camefrom ? namenode->divert->camefrom->name : "<none>",
        namenode->divert->pkgset ? namenode->divert->pkgset->name : "<none>",
        fnn->name);

  return fnn;
}

/**
 * Verify that some programs can be found in the PATH.
 */
void checkpath(void) {
  static const char *const prog_list[] = {
    DPKG_DEFAULT_SHELL,
    RM,
    TAR,
    DIFF,
    BACKEND,
    /* macOS uses dyld (Mach-O) instead of ld.so (ELF), and does not have
     * an ldconfig. */
#if defined(__APPLE__) && defined(__MACH__)
    "update_dyld_shared_cache",
#elif defined(__GLIBC__) || defined(__UCLIBC__) || \
      defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    "ldconfig",
#endif
#if BUILD_START_STOP_DAEMON
    "start-stop-daemon",
#endif
    NULL
  };

  const char *const *prog;
  int warned= 0;

  for (prog = prog_list; *prog; prog++) {
    if (!command_in_path(*prog)) {
      warning(_("'%s' not found in PATH or not executable"), *prog);
      warned++;
    }
  }

  if (warned)
    forcibleerr(FORCE_BAD_PATH,
                P_("%d expected program not found in PATH or not executable\n%s",
                   "%d expected programs not found in PATH or not executable\n%s",
                   warned),
                warned, _("Note: root's PATH should usually contain "
                          "/usr/local/sbin, /usr/sbin and /sbin"));
}

bool
ignore_depends(const struct pkginfo *pkg)
{
  struct pkg_list *id;
  for (id= ignoredependss; id; id= id->next)
    if (id->pkg == pkg)
      return true;
  return false;
}

static bool
ignore_depends_possi(struct deppossi *possi)
{
  struct deppossi_pkg_iterator *possi_iter;
  struct pkginfo *pkg;

  possi_iter = deppossi_pkg_iter_new(possi, wpb_installed);
  while ((pkg = deppossi_pkg_iter_next(possi_iter))) {
    if (ignore_depends(pkg)) {
      deppossi_pkg_iter_free(possi_iter);
      return true;
    }
  }
  deppossi_pkg_iter_free(possi_iter);

  return false;
}

bool
force_depends(struct deppossi *possi)
{
  return in_force(FORCE_DEPENDS) ||
         ignore_depends_possi(possi) ||
         ignore_depends(possi->up->up);
}

bool
force_breaks(struct deppossi *possi)
{
  return in_force(FORCE_BREAKS) ||
         ignore_depends_possi(possi) ||
         ignore_depends(possi->up->up);
}

bool
force_conflicts(struct deppossi *possi)
{
  return in_force(FORCE_CONFLICTS);
}

void clear_istobes(void) {
  struct pkg_hash_iter *iter;
  struct pkginfo *pkg;

  iter = pkg_hash_iter_new();
  while ((pkg = pkg_hash_iter_next_pkg(iter)) != NULL) {
    ensure_package_clientdata(pkg);
    pkg->clientdata->istobe = PKG_ISTOBE_NORMAL;
    pkg->clientdata->replacingfilesandsaid= 0;
  }
  pkg_hash_iter_free(iter);
}

/*
 * Returns true if the directory contains conffiles belonging to pkg,
 * false otherwise.
 */
bool
dir_has_conffiles(struct fsys_namenode *file, struct pkginfo *pkg)
{
  struct conffile *conff;
  size_t namelen;

  debug(dbg_veryverbose, "dir_has_conffiles '%s' (from %s)", file->name,
        pkg_name(pkg, pnaw_always));
  namelen = strlen(file->name);
  for (conff= pkg->installed.conffiles; conff; conff= conff->next) {
      if (conffile_is_disappearing(conff))
        continue;
      if (strncmp(file->name, conff->name, namelen) == 0 &&
          strlen(conff->name) > namelen && conff->name[namelen] == '/') {
	debug(dbg_veryverbose, "directory %s has conffile %s from %s",
	      file->name, conff->name, pkg_name(pkg, pnaw_always));
	return true;
      }
  }
  debug(dbg_veryverbose, "dir_has_conffiles no");
  return false;
}

/*
 * Returns true if the file is used by packages other than pkg,
 * false otherwise.
 */
bool
dir_is_used_by_others(struct fsys_namenode *file, struct pkginfo *pkg)
{
  struct fsys_node_pkgs_iter *iter;
  struct pkginfo *other_pkg;

  debug(dbg_veryverbose, "dir_is_used_by_others '%s' (except %s)", file->name,
        pkg ? pkg_name(pkg, pnaw_always) : "<none>");

  iter = fsys_node_pkgs_iter_new(file);
  while ((other_pkg = fsys_node_pkgs_iter_next(iter))) {
    debug(dbg_veryverbose, "dir_is_used_by_others considering %s ...",
          pkg_name(other_pkg, pnaw_always));
    if (other_pkg == pkg)
      continue;

    fsys_node_pkgs_iter_free(iter);
    debug(dbg_veryverbose, "dir_is_used_by_others yes");
    return true;
  }
  fsys_node_pkgs_iter_free(iter);

  debug(dbg_veryverbose, "dir_is_used_by_others no");
  return false;
}

/*
 * Returns true if the file is used by pkg, false otherwise.
 */
bool
dir_is_used_by_pkg(struct fsys_namenode *file, struct pkginfo *pkg,
                   struct fsys_namenode_list *list)
{
  struct fsys_namenode_list *node;
  size_t namelen;

  debug(dbg_veryverbose, "dir_is_used_by_pkg '%s' (by %s)",
        file->name, pkg ? pkg_name(pkg, pnaw_always) : "<none>");

  namelen = strlen(file->name);

  for (node = list; node; node = node->next) {
    debug(dbg_veryverbose, "dir_is_used_by_pkg considering %s ...",
          node->namenode->name);

    if (strncmp(file->name, node->namenode->name, namelen) == 0 &&
        strlen(node->namenode->name) > namelen &&
        node->namenode->name[namelen] == '/') {
      debug(dbg_veryverbose, "dir_is_used_by_pkg yes");
      return true;
    }
  }

  debug(dbg_veryverbose, "dir_is_used_by_pkg no");

  return false;
}

/**
 * Returns whether the conffile is disappearing, because it is obsolete
 * or marked for removal on upgrade.
 */
bool
conffile_is_disappearing(struct conffile *conff)
{
  return conff->flags & (CONFFILE_OBSOLETE | CONFFILE_REMOVE_ON_UPGRADE);
}

/**
 * Mark a conffile as obsolete.
 *
 * @param pkg		The package owning the conffile.
 * @param namenode	The namenode for the obsolete conffile.
 */
void
conffile_mark_obsolete(struct pkginfo *pkg, struct fsys_namenode *namenode)
{
  struct conffile *conff;

  for (conff = pkg->installed.conffiles; conff; conff = conff->next) {
    if (strcmp(conff->name, namenode->name) == 0) {
      debug(dbg_conff, "marking %s conffile %s as obsolete",
            pkg_name(pkg, pnaw_always), conff->name);
      conff->flags |= CONFFILE_OBSOLETE;
      return;
    }
  }
}

/**
 * Mark all package conffiles as old.
 *
 * @param pkg		The package owning the conffiles.
 */
void
pkg_conffiles_mark_old(struct pkginfo *pkg)
{
  const struct conffile *conff;

  for (conff = pkg->installed.conffiles; conff; conff = conff->next) {
    struct fsys_namenode *namenode;

    namenode = fsys_hash_find_node(conff->name, FHFF_NONE); /* XXX */
    namenode->flags |= FNNF_OLD_CONFF;
    if (!namenode->oldhash)
      namenode->oldhash = conff->hash;
    debug(dbg_conffdetail, "%s '%s' namenode '%s' flags %o", __func__,
          conff->name, namenode->name, namenode->flags);
  }
}

void
log_action(const char *action, struct pkginfo *pkg, struct pkgbin *pkgbin)
{
  log_message("%s %s %s %s", action, pkgbin_name(pkg, pkgbin, pnaw_always),
              versiondescribe_c(&pkg->installed.version, vdew_nonambig),
              versiondescribe_c(&pkg->available.version, vdew_nonambig));
  statusfd_send("processing: %s: %s", action,
                pkgbin_name(pkg, pkgbin, pnaw_nonambig));
}
