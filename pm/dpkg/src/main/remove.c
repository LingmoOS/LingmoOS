/*
 * dpkg - main program for package management
 * remove.c - functionality for removing packages
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
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg.h>
#include <dpkg/path.h>
#include <dpkg/dir.h>
#include <dpkg/options.h>
#include <dpkg/triglib.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>

#include "main.h"

/*
 * pkgdepcheck may be a virtual pkg.
 */
static void checkforremoval(struct pkginfo *pkgtoremove,
                            struct pkgset *pkgdepcheck,
                            enum dep_check *rokp, struct varbuf *raemsgs)
{
  struct deppossi *possi;
  struct pkginfo *depender;
  enum dep_check ok;
  struct varbuf_state raemsgs_state;

  for (possi = pkgdepcheck->depended.installed; possi; possi = possi->rev_next) {
    if (possi->up->type != dep_depends && possi->up->type != dep_predepends) continue;
    depender= possi->up->up;
    debug(dbg_depcon, "checking depending package '%s'",
          pkg_name(depender, pnaw_always));
    if (depender->status < PKG_STAT_UNPACKED)
      continue;
    if (ignore_depends(depender)) {
      debug(dbg_depcon, "ignoring depending package '%s'",
            pkg_name(depender, pnaw_always));
      continue;
    }
    if (dependtry >= DEPEND_TRY_CYCLES) {
      if (findbreakcycle(pkgtoremove))
        sincenothing = 0;
    }
    varbuf_snapshot(raemsgs, &raemsgs_state);
    ok= dependencies_ok(depender,pkgtoremove,raemsgs);
    if (ok == DEP_CHECK_HALT &&
        depender->clientdata &&
        depender->clientdata->istobe == PKG_ISTOBE_REMOVE)
      ok = DEP_CHECK_DEFER;
    if (ok == DEP_CHECK_DEFER)
      /* Don't burble about reasons for deferral. */
      varbuf_rollback(&raemsgs_state);
    if (ok < *rokp) *rokp= ok;
  }
}

void deferred_remove(struct pkginfo *pkg) {
  struct varbuf raemsgs = VARBUF_INIT;
  struct dependency *dep;
  enum dep_check rok;

  debug(dbg_general, "deferred_remove package %s",
        pkg_name(pkg, pnaw_always));

  if (!f_pending && pkg->want != PKG_WANT_UNKNOWN) {
    if (cipaction->arg_int == act_purge)
      pkg_set_want(pkg, PKG_WANT_PURGE);
    else
      pkg_set_want(pkg, PKG_WANT_DEINSTALL);

    if (f_act)
      modstatdb_note(pkg);
  }

  ensure_package_clientdata(pkg);

  if (pkg->status == PKG_STAT_NOTINSTALLED) {
    sincenothing = 0;
    warning(_("ignoring request to remove %.250s which isn't installed"),
            pkg_name(pkg, pnaw_nonambig));
    pkg->clientdata->istobe = PKG_ISTOBE_NORMAL;
    return;
  } else if (!f_pending &&
             pkg->status == PKG_STAT_CONFIGFILES &&
             cipaction->arg_int != act_purge) {
    sincenothing = 0;
    warning(_("ignoring request to remove %.250s, only the config\n"
              " files of which are on the system; use --purge to remove them too"),
            pkg_name(pkg, pnaw_nonambig));
    pkg->clientdata->istobe = PKG_ISTOBE_NORMAL;
    return;
  }

  if (pkg->status != PKG_STAT_CONFIGFILES) {
    if (pkg->installed.essential)
      forcibleerr(FORCE_REMOVE_ESSENTIAL,
                  _("this is an essential package; it should not be removed"));
    if (pkg->installed.is_protected)
      forcibleerr(FORCE_REMOVE_PROTECTED,
                  _("this is a protected package; it should not be removed"));
  }

  debug(dbg_general, "checking dependencies for remove '%s'",
        pkg_name(pkg, pnaw_always));
  rok = DEP_CHECK_OK;
  checkforremoval(pkg, pkg->set, &rok, &raemsgs);
  for (dep= pkg->installed.depends; dep; dep= dep->next) {
    if (dep->type != dep_provides) continue;
    debug(dbg_depcon, "checking virtual package '%s'", dep->list->ed->name);
    checkforremoval(pkg, dep->list->ed, &rok, &raemsgs);
  }

  if (rok == DEP_CHECK_DEFER) {
    varbuf_destroy(&raemsgs);
    pkg->clientdata->istobe = PKG_ISTOBE_REMOVE;
    enqueue_package(pkg);
    return;
  } else if (rok == DEP_CHECK_HALT) {
    sincenothing= 0;
    notice(_("dependency problems prevent removal of %s:\n%s"),
            pkg_name(pkg, pnaw_nonambig), varbuf_str(&raemsgs));
    ohshit(_("dependency problems - not removing"));
  } else if (raemsgs.used) {
    notice(_("%s: dependency problems, but removing anyway as you requested:\n%s"),
            pkg_name(pkg, pnaw_nonambig), varbuf_str(&raemsgs));
  }
  varbuf_destroy(&raemsgs);
  sincenothing= 0;

  if (pkg->eflag & PKG_EFLAG_REINSTREQ)
    forcibleerr(FORCE_REMOVE_REINSTREQ,
                _("package is in a very bad inconsistent state; you should\n"
                  " reinstall it before attempting a removal"));

  ensure_allinstfiles_available();
  fsys_hash_init();

  if (!f_act) {
    printf(_("Would remove or purge %s (%s) ...\n"),
           pkg_name(pkg, pnaw_nonambig),
           versiondescribe(&pkg->installed.version, vdew_nonambig));
    pkg_set_status(pkg, PKG_STAT_NOTINSTALLED);
    pkg->clientdata->istobe = PKG_ISTOBE_NORMAL;
    return;
  }

  pkg_conffiles_mark_old(pkg);

  /* Only print and log removal action once. This avoids duplication when
   * using --remove and --purge in sequence. */
  if (pkg->status > PKG_STAT_CONFIGFILES) {
    printf(_("Removing %s (%s) ...\n"), pkg_name(pkg, pnaw_nonambig),
           versiondescribe(&pkg->installed.version, vdew_nonambig));
    log_action("remove", pkg, &pkg->installed);
  }

  trig_activate_packageprocessing(pkg);
  if (pkg->status >= PKG_STAT_HALFCONFIGURED) {
    static enum pkgstatus oldpkgstatus;

    oldpkgstatus= pkg->status;
    pkg_set_status(pkg, PKG_STAT_HALFCONFIGURED);
    modstatdb_note(pkg);
    push_cleanup(cu_prermremove, ~ehflag_normaltidy, 2,
                 (void *)pkg, (void *)&oldpkgstatus);
    maintscript_installed(pkg, PRERMFILE, "pre-removal", "remove", NULL);

    /* Will turn into ‘half-installed’ soon ... */
    pkg_set_status(pkg, PKG_STAT_UNPACKED);
  }

  removal_bulk(pkg);
}

static void
push_leftover(struct fsys_namenode_list **leftoverp,
              struct fsys_namenode *namenode)
{
  struct fsys_namenode_list *newentry;

  newentry = nfmalloc(sizeof(*newentry));
  newentry->next= *leftoverp;
  newentry->namenode= namenode;
  *leftoverp= newentry;
}

static void
removal_bulk_remove_file(const char *filename, const char *filetype)
{
  /* We need the postrm and list files for --purge. */
  if (strcmp(filetype, LISTFILE) == 0 ||
      strcmp(filetype, POSTRMFILE) == 0)
    return;

  debug(dbg_stupidlyverbose, "removal_bulk info not postrm or list");

  if (unlink(filename))
    ohshite(_("unable to delete control info file '%.250s'"), filename);

  debug(dbg_scripts, "removal_bulk info unlinked %s", filename);
}

static bool
removal_bulk_file_is_shared(struct pkginfo *pkg, struct fsys_namenode *namenode)
{
  struct fsys_node_pkgs_iter *iter;
  struct pkginfo *otherpkg;
  bool shared = false;

  if (pkgset_installed_instances(pkg->set) <= 1)
    return false;

  iter = fsys_node_pkgs_iter_new(namenode);
  while ((otherpkg = fsys_node_pkgs_iter_next(iter))) {
    if (otherpkg == pkg)
      continue;
    if (otherpkg->set != pkg->set)
      continue;

    debug(dbg_eachfiledetail, "removal_bulk file shared with %s, skipping",
          pkg_name(otherpkg, pnaw_always));
    shared = true;
    break;
  }
  fsys_node_pkgs_iter_free(iter);

  return shared;
}

static void
removal_bulk_remove_files(struct pkginfo *pkg)
{
  struct fsys_hash_rev_iter rev_iter;
  struct fsys_namenode_list *leftover;
  struct fsys_namenode *namenode;
  static struct varbuf fnvb;
  struct varbuf_state fnvb_state;
  struct stat stab;

    pkg_set_status(pkg, PKG_STAT_HALFINSTALLED);
    modstatdb_note(pkg);
    push_checkpoint(~ehflag_bombout, ehflag_normaltidy);

    fsys_hash_rev_iter_init(&rev_iter, pkg->files);
    leftover = NULL;
    while ((namenode = fsys_hash_rev_iter_next(&rev_iter))) {
      struct fsys_namenode *usenode;
      bool is_dir;

      debug(dbg_eachfile, "removal_bulk '%s' flags=%o",
            namenode->name, namenode->flags);

      usenode = namenodetouse(namenode, pkg, &pkg->installed);

      varbuf_set_str(&fnvb, dpkg_fsys_get_dir());
      varbuf_add_str(&fnvb, usenode->name);
      varbuf_snapshot(&fnvb, &fnvb_state);

      is_dir = stat(fnvb.buf, &stab) == 0 && S_ISDIR(stab.st_mode);

      /* A pkgset can share files between its instances that we
       * don't want to remove, we just want to forget them. This
       * applies to shared conffiles too. */
      if (!is_dir && removal_bulk_file_is_shared(pkg, namenode))
        continue;

      /* Non-shared conffiles are kept. */
      if (namenode->flags & FNNF_OLD_CONFF) {
        push_leftover(&leftover, namenode);
        continue;
      }

      if (is_dir) {
        debug(dbg_eachfiledetail, "removal_bulk is a directory");
        /* Only delete a directory or a link to one if we're the only
         * package which uses it. Other files should only be listed
         * in this package (but we don't check). */
        if (dir_has_conffiles(namenode, pkg)) {
	  push_leftover(&leftover,namenode);
	  continue;
	}
        if (dir_is_used_by_pkg(namenode, pkg, leftover)) {
          push_leftover(&leftover, namenode);
          continue;
        }
        if (dir_is_used_by_others(namenode, pkg))
          continue;

        if (strcmp(usenode->name, "/.") == 0) {
          debug(dbg_eachfiledetail,
                "removal_bulk '%s' root directory, cannot remove", fnvb.buf);
          push_leftover(&leftover, namenode);
          continue;
        }
      }

      trig_path_activate(usenode, pkg);

      varbuf_rollback(&fnvb_state);
      varbuf_add_str(&fnvb, DPKGTEMPEXT);
      debug(dbg_eachfiledetail, "removal_bulk cleaning temp '%s'", fnvb.buf);
      path_remove_tree(fnvb.buf);

      varbuf_rollback(&fnvb_state);
      varbuf_add_str(&fnvb, DPKGNEWEXT);
      debug(dbg_eachfiledetail, "removal_bulk cleaning new '%s'", fnvb.buf);
      path_remove_tree(fnvb.buf);

      varbuf_rollback(&fnvb_state);

      debug(dbg_eachfiledetail, "removal_bulk removing '%s'", fnvb.buf);
      if (!rmdir(fnvb.buf) || errno == ENOENT || errno == ELOOP) continue;
      if (errno == ENOTEMPTY || errno == EEXIST) {
        debug(dbg_eachfiledetail,
              "removal_bulk '%s' was not empty, will try again later",
	      fnvb.buf);
        push_leftover(&leftover,namenode);
        continue;
      } else if (errno == EBUSY || errno == EPERM) {
        warning(_("while removing %.250s, unable to remove directory '%.250s': "
                  "%s - directory may be a mount point?"),
                pkg_name(pkg, pnaw_nonambig), namenode->name, strerror(errno));
        push_leftover(&leftover,namenode);
        continue;
      }
      if (errno != ENOTDIR)
        ohshite(_("cannot remove '%.250s'"), fnvb.buf);
      debug(dbg_eachfiledetail, "removal_bulk unlinking '%s'", fnvb.buf);
      if (secure_unlink(fnvb.buf))
        ohshite(_("unable to securely remove '%.250s'"), fnvb.buf);
    }
    write_filelist_except(pkg, &pkg->installed, leftover, 0);
    maintscript_installed(pkg, POSTRMFILE, "post-removal", "remove", NULL);

    trig_parse_ci(pkg_infodb_get_file(pkg, &pkg->installed, TRIGGERSCIFILE),
                  trig_cicb_interest_delete, NULL, pkg, &pkg->installed);
    trig_file_interests_save();

    debug(dbg_general, "removal_bulk cleaning info directory");
    pkg_infodb_foreach(pkg, &pkg->installed, removal_bulk_remove_file);
    dir_sync_path(pkg_infodb_get_dir());

    pkg_set_status(pkg, PKG_STAT_CONFIGFILES);
    pkg->installed.essential = false;
    pkg->installed.is_protected = false;
    modstatdb_note(pkg);
    push_checkpoint(~ehflag_bombout, ehflag_normaltidy);
}

static void removal_bulk_remove_leftover_dirs(struct pkginfo *pkg) {
  struct fsys_hash_rev_iter rev_iter;
  struct fsys_namenode_list *leftover;
  struct fsys_namenode *namenode;
  static struct varbuf fnvb;
  struct stat stab;

  /* We may have modified this previously. */
  ensure_packagefiles_available(pkg);

  modstatdb_note(pkg);
  push_checkpoint(~ehflag_bombout, ehflag_normaltidy);

  fsys_hash_rev_iter_init(&rev_iter, pkg->files);
  leftover = NULL;
  while ((namenode = fsys_hash_rev_iter_next(&rev_iter))) {
    struct fsys_namenode *usenode;

    debug(dbg_eachfile, "removal_bulk '%s' flags=%o",
          namenode->name, namenode->flags);
    if (namenode->flags & FNNF_OLD_CONFF) {
      /* This can only happen if removal_bulk_remove_configfiles() got
       * interrupted half way. */
      debug(dbg_eachfiledetail, "removal_bulk expecting only left over dirs, "
                                "ignoring conffile '%s'", namenode->name);
      continue;
    }

    usenode = namenodetouse(namenode, pkg, &pkg->installed);

    varbuf_set_str(&fnvb, dpkg_fsys_get_dir());
    varbuf_add_str(&fnvb, usenode->name);

    if (!stat(fnvb.buf,&stab) && S_ISDIR(stab.st_mode)) {
      debug(dbg_eachfiledetail, "removal_bulk is a directory");
      /* Only delete a directory or a link to one if we're the only
       * package which uses it. Other files should only be listed
       * in this package (but we don't check). */
      if (dir_is_used_by_pkg(namenode, pkg, leftover)) {
        push_leftover(&leftover, namenode);
        continue;
      }
      if (dir_is_used_by_others(namenode, pkg))
        continue;

      if (strcmp(usenode->name, "/.") == 0) {
        debug(dbg_eachfiledetail,
              "removal_bulk '%s' root directory, cannot remove", fnvb.buf);
        push_leftover(&leftover, namenode);
        continue;
      }
    }

    trig_path_activate(usenode, pkg);

    debug(dbg_eachfiledetail, "removal_bulk removing '%s'", fnvb.buf);
    if (!rmdir(fnvb.buf) || errno == ENOENT || errno == ELOOP) continue;
    if (errno == ENOTEMPTY || errno == EEXIST) {
      warning(_("while removing %.250s, directory '%.250s' not empty so not removed"),
              pkg_name(pkg, pnaw_nonambig), namenode->name);
      push_leftover(&leftover,namenode);
      continue;
    } else if (errno == EBUSY || errno == EPERM) {
      warning(_("while removing %.250s, unable to remove directory '%.250s': "
                "%s - directory may be a mount point?"),
              pkg_name(pkg, pnaw_nonambig), namenode->name, strerror(errno));
      push_leftover(&leftover,namenode);
      continue;
    }
    if (errno != ENOTDIR)
      ohshite(_("cannot remove '%.250s'"), fnvb.buf);

    if (lstat(fnvb.buf, &stab) == 0 && S_ISLNK(stab.st_mode)) {
      debug(dbg_eachfiledetail, "removal_bulk is a symlink to a directory");

      if (unlink(fnvb.buf))
        ohshite(_("cannot remove '%.250s'"), fnvb.buf);

      continue;
    }

    push_leftover(&leftover,namenode);
  }
  write_filelist_except(pkg, &pkg->installed, leftover, 0);

  modstatdb_note(pkg);
  push_checkpoint(~ehflag_bombout, ehflag_normaltidy);
}

static void removal_bulk_remove_configfiles(struct pkginfo *pkg) {
  static const char *const removeconffexts[] = { REMOVECONFFEXTS, NULL };
  int rc;
  int conffnameused, conffbasenamelen;
  char *conffbasename;
  struct conffile *conff, **lconffp;
  struct fsys_namenode_list *searchfile;
  DIR *dsd;
  struct dirent *de;
  char *p;
  const char *const *ext;

    printf(_("Purging configuration files for %s (%s) ...\n"),
           pkg_name(pkg, pnaw_nonambig),
           versiondescribe(&pkg->installed.version, vdew_nonambig));
    log_action("purge", pkg, &pkg->installed);
    trig_activate_packageprocessing(pkg);

    /* We may have modified this above. */
    ensure_packagefiles_available(pkg);

    /* We're about to remove the configuration, so remove the note
     * about which version it was ... */
    dpkg_version_blank(&pkg->configversion);
    modstatdb_note(pkg);

    /* Remove from our list any conffiles that aren't ours any more or
     * are involved in diversions, except if we are the package doing the
     * diverting. */
    for (lconffp = &pkg->installed.conffiles; (conff = *lconffp) != NULL; ) {
      for (searchfile = pkg->files;
           searchfile && strcmp(searchfile->namenode->name,conff->name);
           searchfile= searchfile->next);
      if (!searchfile) {
        debug(dbg_conff, "removal_bulk conffile not ours any more '%s'",
              conff->name);
        *lconffp= conff->next;
      } else if (searchfile->namenode->divert &&
                 (searchfile->namenode->divert->camefrom ||
                  (searchfile->namenode->divert->useinstead &&
                   searchfile->namenode->divert->pkgset != pkg->set))) {
        debug(dbg_conff, "removal_bulk conffile '%s' ignored due to diversion",
              conff->name);
        *lconffp= conff->next;
      } else {
        debug(dbg_conffdetail, "removal_bulk set to new conffile '%s'",
              conff->name);
        conff->hash = NEWCONFFILEFLAG;
        lconffp= &conff->next;
      }
    }
    modstatdb_note(pkg);

    for (conff= pkg->installed.conffiles; conff; conff= conff->next) {
      struct fsys_namenode *namenode, *usenode;
    static struct varbuf fnvb, removevb;
      struct varbuf_state removevb_state;

      if (conff->flags & CONFFILE_OBSOLETE) {
	debug(dbg_conffdetail, "removal_bulk conffile obsolete %s",
	      conff->name);
      }
      varbuf_reset(&fnvb);
      rc = conffderef(pkg, &fnvb, conff->name);
      if (rc < 0) {
        debug(dbg_conffdetail, "removal_bulk conffile '%s' (rc < %d)",
              conff->name, rc);
        continue;
      } else {
        debug(dbg_conffdetail, "removal_bulk conffile '%s' (deref '%s')",
              conff->name, fnvb.buf);
      }

      namenode = fsys_hash_find_node(conff->name, FHFF_NONE);
      usenode = namenodetouse(namenode, pkg, &pkg->installed);

      trig_path_activate(usenode, pkg);

      conffnameused = fnvb.used;
      if (unlink(fnvb.buf) && errno != ENOENT && errno != ENOTDIR)
        ohshite(_("cannot remove old config file '%.250s' (= '%.250s')"),
                conff->name, fnvb.buf);
      p= strrchr(fnvb.buf,'/'); if (!p) continue;
      *p = '\0';
      varbuf_reset(&removevb);
      varbuf_add_dir(&removevb, fnvb.buf);
      varbuf_snapshot(&removevb, &removevb_state);

      dsd= opendir(removevb.buf);
      if (!dsd) {
        int e=errno;
        debug(dbg_conffdetail, "removal_bulk conffile no dsd %s %s",
              fnvb.buf, strerror(e)); errno= e;
        if (errno == ENOENT || errno == ENOTDIR) continue;
        ohshite(_("cannot read config file directory '%.250s' (from '%.250s')"),
                fnvb.buf, conff->name);
      }
      debug(dbg_conffdetail, "removal_bulk conffile cleaning dsd %s", fnvb.buf);
      push_cleanup(cu_closedir, ~0, 1, (void *)dsd);
      *p= '/';
      conffbasenamelen= strlen(++p);
      conffbasename= fnvb.buf+conffnameused-conffbasenamelen;
      while ((de = readdir(dsd)) != NULL) {
        debug(dbg_stupidlyverbose, "removal_bulk conffile dsd entry='%s'"
              " conffbasename='%s' conffnameused=%d conffbasenamelen=%d",
              de->d_name, conffbasename, conffnameused, conffbasenamelen);
        if (strncmp(de->d_name, conffbasename, conffbasenamelen) == 0) {
          debug(dbg_stupidlyverbose, "removal_bulk conffile dsd entry starts right");
          for (ext= removeconffexts; *ext; ext++)
            if (strcmp(*ext, de->d_name + conffbasenamelen) == 0)
              goto yes_remove;
          p= de->d_name+conffbasenamelen;
          if (*p++ == '~') {
            while (*p && c_isdigit(*p))
              p++;
            if (*p == '~' && !*++p) goto yes_remove;
          }
        }
        debug(dbg_stupidlyverbose, "removal_bulk conffile dsd entry starts wrong");
        if (de->d_name[0] == '#' &&
            strncmp(de->d_name + 1, conffbasename, conffbasenamelen) == 0 &&
            strcmp(de->d_name + 1 + conffbasenamelen, "#") == 0)
          goto yes_remove;
        debug(dbg_stupidlyverbose, "removal_bulk conffile dsd entry not it");
        continue;
      yes_remove:
        varbuf_rollback(&removevb_state);
        varbuf_add_str(&removevb, de->d_name);
        debug(dbg_conffdetail, "removal_bulk conffile dsd entry removing '%s'",
              removevb.buf);
        if (unlink(removevb.buf) && errno != ENOENT && errno != ENOTDIR)
          ohshite(_("cannot remove old backup config file '%.250s' (of '%.250s')"),
                  removevb.buf, conff->name);
      }
      pop_cleanup(ehflag_normaltidy); /* closedir */
    }

    /* Remove the conffiles from the file list file. */
    write_filelist_except(pkg, &pkg->installed, pkg->files,
                          FNNF_OLD_CONFF);

    pkg->installed.conffiles = NULL;
    modstatdb_note(pkg);

    maintscript_installed(pkg, POSTRMFILE, "post-removal", "purge", NULL);
}

/*
 * This is used both by deferred_remove() in this file, and at the end of
 * process_archive() in archives.c if it needs to finish removing a
 * conflicting package.
 */
void removal_bulk(struct pkginfo *pkg) {
  bool foundpostrm;

  debug(dbg_general, "removal_bulk package %s", pkg_name(pkg, pnaw_always));

  if (pkg->status == PKG_STAT_HALFINSTALLED ||
      pkg->status == PKG_STAT_UNPACKED) {
    removal_bulk_remove_files(pkg);
  }

  foundpostrm = pkg_infodb_has_file(pkg, &pkg->installed, POSTRMFILE);

  debug(dbg_general, "removal_bulk purging? foundpostrm=%d",foundpostrm);

  if (!foundpostrm && !pkg->installed.conffiles) {
    /* If there are no config files and no postrm script then we
     * go straight into ‘purge’.  */
    debug(dbg_general, "removal_bulk no postrm, no conffiles, purging");

    pkg_set_want(pkg, PKG_WANT_PURGE);
    dpkg_version_blank(&pkg->configversion);
  } else if (pkg->want == PKG_WANT_PURGE) {
    removal_bulk_remove_configfiles(pkg);
  }

  /* I.e., either of the two branches above. */
  if (pkg->want == PKG_WANT_PURGE) {
    const char *filename;

    /* Retry empty directories, and warn on any leftovers that aren't. */
    removal_bulk_remove_leftover_dirs(pkg);

    filename = pkg_infodb_get_file(pkg, &pkg->installed, LISTFILE);
    debug(dbg_general, "removal_bulk purge done, removing list '%s'",
          filename);
    if (unlink(filename) && errno != ENOENT)
      ohshite(_("cannot remove old files list"));

    filename = pkg_infodb_get_file(pkg, &pkg->installed, POSTRMFILE);
    debug(dbg_general, "removal_bulk purge done, removing postrm '%s'",
          filename);
    if (unlink(filename) && errno != ENOENT)
      ohshite(_("can't remove old postrm script"));

    pkg_set_status(pkg, PKG_STAT_NOTINSTALLED);
    pkg_set_want(pkg, PKG_WANT_UNKNOWN);

    /* This will mess up reverse links, but if we follow them
     * we won't go back because pkg->status is PKG_STAT_NOTINSTALLED. */
    pkgbin_blank(&pkg->installed);
  }

  pkg_reset_eflags(pkg);
  modstatdb_note(pkg);

  debug(dbg_general, "removal done");
}
