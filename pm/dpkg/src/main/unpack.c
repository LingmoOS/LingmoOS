/*
 * dpkg - main program for package management
 * unpack.c - .deb archive unpacking
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006-2015 Guillem Jover <guillem@debian.org>
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
#include <string.h>
#include <time.h>
#include <utime.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg.h>
#include <dpkg/pkg-queue.h>
#include <dpkg/path.h>
#include <dpkg/command.h>
#include <dpkg/buffer.h>
#include <dpkg/subproc.h>
#include <dpkg/dir.h>
#include <dpkg/tarfn.h>
#include <dpkg/options.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>
#include <dpkg/triglib.h>

#include "file-match.h"
#include "main.h"
#include "archives.h"

static const char *
summarize_filename(const char *filename)
{
  const char *pfilename;
  char *pfilenamebuf;

  for (pfilename = filename;
       pfilename && strlen(pfilename) > 30 && strchr(pfilename, '/') != NULL;
       pfilename++)
    pfilename = strchr(pfilename, '/');

  if (pfilename && pfilename != filename) {
    pfilenamebuf = nfmalloc(strlen(pfilename) + 5);
    sprintf(pfilenamebuf, _(".../%s"), pfilename);
    pfilename = pfilenamebuf;
  } else {
    pfilename = filename;
  }

  return pfilename;
}

static bool
deb_reassemble(const char **filename, const char **pfilename)
{
  static char *reasmbuf = NULL;
  struct stat stab;
  int status;
  pid_t pid;

  if (!reasmbuf)
    reasmbuf = dpkg_db_get_path(REASSEMBLETMP);
  if (unlink(reasmbuf) && errno != ENOENT)
    ohshite(_("error ensuring '%.250s' doesn't exist"), reasmbuf);

  push_cleanup(cu_pathname, ~0, 1, (void *)reasmbuf);

  pid = subproc_fork();
  if (!pid) {
    execlp(SPLITTER, SPLITTER, "-Qao", reasmbuf, *filename, NULL);
    ohshite(_("unable to execute %s (%s)"),
            _("split package reassembly"), SPLITTER);
  }
  status = subproc_reap(pid, SPLITTER, SUBPROC_RETERROR);
  switch (status) {
  case 0:
    /* It was a part - is it complete? */
    if (!stat(reasmbuf, &stab)) {
      /* Yes. */
      *filename = reasmbuf;
      *pfilename = _("reassembled package file");
      break;
    } else if (errno == ENOENT) {
      /* No. That's it, we skip it. */
      return false;
    }
  case 1:
    /* No, it wasn't a part. */
    break;
  default:
    ohshit(_("subprocess %s returned error exit status %d"), SPLITTER, status);
  }

  return true;
}

static void
deb_verify(const char *filename)
{
  pid_t pid;

  /* We have to check on every unpack, in case the debsig-verify package
   * gets installed or removed. */
  if (!command_in_path(DEBSIGVERIFY))
    return;

  printf(_("Authenticating %s ...\n"), filename);
  fflush(stdout);
  pid = subproc_fork();
  if (!pid) {
    execlp(DEBSIGVERIFY, DEBSIGVERIFY, "-q", filename, NULL);
    ohshite(_("unable to execute %s (%s)"),
            _("package signature verification"), DEBSIGVERIFY);
  } else {
    int status;

    status = subproc_reap(pid, "debsig-verify", SUBPROC_NOCHECK);
    if (!(WIFEXITED(status) && WEXITSTATUS(status) == 0)) {
      if (!in_force(FORCE_BAD_VERIFY))
        ohshit(_("verification on package %s failed!"), filename);
      else
        notice(_("verification on package %s failed; "
                 "but installing anyway as you requested"), filename);
    } else {
      printf(_("passed\n"));
    }
  }
}

static char *
get_control_dir(char *cidir)
{
  if (!f_act) {
    char *tmpdir;

    tmpdir = mkdtemp(path_make_temp_template("dpkg"));
    if (tmpdir == NULL)
      ohshite(_("unable to create temporary directory"));

    cidir = m_realloc(cidir, strlen(tmpdir) + MAXCONTROLFILENAME + 10);

    strcpy(cidir, tmpdir);

    free(tmpdir);
  } else {
    const char *admindir;

    admindir = dpkg_db_get_dir();

    /* The admindir length is always constant on a dpkg execution run. */
    if (cidir == NULL)
      cidir = m_malloc(strlen(admindir) + sizeof(CONTROLDIRTMP) +
                       MAXCONTROLFILENAME + 10);

    /* We want it to be on the same filesystem so that we can
     * use rename(2) to install the postinst &c. */
    strcpy(cidir, admindir);
    strcat(cidir, "/" CONTROLDIRTMP);

    /* Make sure the control information directory is empty. */
    path_remove_tree(cidir);
  }

  strcat(cidir, "/");

  return cidir;
}

static void
pkg_check_depcon(struct pkginfo *pkg, const char *pfilename)
{
  struct dependency *dsearch;
  struct deppossi *psearch;
  struct pkginfo *fixbytrigaw;
  static struct varbuf depprobwhy;

  /* Check if anything is installed that we conflict with, or not installed
   * that we need. */
  ensure_package_clientdata(pkg);
  pkg->clientdata->istobe = PKG_ISTOBE_INSTALLNEW;

  for (dsearch = pkg->available.depends; dsearch; dsearch = dsearch->next) {
    switch (dsearch->type) {
    case dep_conflicts:
      /* Look for things we conflict with. */
      check_conflict(dsearch, pkg, pfilename);
      break;
    case dep_breaks:
      /* Look for things we break. */
      check_breaks(dsearch, pkg, pfilename);
      break;
    case dep_provides:
      /* Look for things that conflict with what we provide. */
      for (psearch = dsearch->list->ed->depended.installed;
           psearch;
           psearch = psearch->rev_next) {
        if (psearch->up->type != dep_conflicts)
          continue;
        check_conflict(psearch->up, pkg, pfilename);
      }
      break;
    case dep_suggests:
    case dep_recommends:
    case dep_depends:
    case dep_replaces:
    case dep_enhances:
      /* Ignore these here. */
      break;
    case dep_predepends:
      if (!depisok(dsearch, &depprobwhy, NULL, &fixbytrigaw, true)) {
        if (fixbytrigaw) {
          while (fixbytrigaw->trigaw.head)
            trigproc(fixbytrigaw->trigaw.head->pend, TRIGPROC_REQUIRED);
        } else {
          notice(_("regarding %s containing %s, pre-dependency problem:\n%s"),
                 pfilename, pkgbin_name(pkg, &pkg->available, pnaw_nonambig),
                 varbuf_str(&depprobwhy));
          if (!force_depends(dsearch->list))
            ohshit(_("pre-dependency problem - not installing %.250s"),
                   pkgbin_name(pkg, &pkg->available, pnaw_nonambig));
          warning(_("ignoring pre-dependency problem!"));
        }
      }
    }
  }

  /* Look for things that conflict with us. */
  for (psearch = pkg->set->depended.installed; psearch; psearch = psearch->rev_next) {
    if (psearch->up->type != dep_conflicts)
      continue;

    check_conflict(psearch->up, pkg, pfilename);
  }
}

static void
pkg_deconfigure_others(struct pkginfo *pkg)
{
  struct pkg_deconf_list *deconpil;

  for (deconpil = deconfigure; deconpil; deconpil = deconpil->next) {
    struct pkginfo *removing = deconpil->pkg_removal;

    if (deconpil->reason == PKG_WANT_DEINSTALL) {
      printf(_("De-configuring %s (%s), to allow removal of %s (%s) ...\n"),
             pkg_name(deconpil->pkg, pnaw_nonambig),
             versiondescribe(&deconpil->pkg->installed.version, vdew_nonambig),
             pkg_name(removing, pnaw_nonambig),
             versiondescribe(&removing->installed.version, vdew_nonambig));
    } else if (deconpil->reason == PKG_WANT_INSTALL) {
      printf(_("De-configuring %s (%s), to allow installation of %s (%s) ...\n"),
             pkg_name(deconpil->pkg, pnaw_nonambig),
             versiondescribe(&deconpil->pkg->installed.version, vdew_nonambig),
             pkg_name(pkg, pnaw_nonambig),
             versiondescribe(&pkg->available.version, vdew_nonambig));
    } else {
      printf(_("De-configuring %s (%s), to allow configuration of %s (%s) ...\n"),
             pkg_name(deconpil->pkg, pnaw_nonambig),
             versiondescribe(&deconpil->pkg->installed.version, vdew_nonambig),
             pkg_name(pkg, pnaw_nonambig),
             versiondescribe(&pkg->available.version, vdew_nonambig));
    }

    trig_activate_packageprocessing(deconpil->pkg);
    pkg_set_status(deconpil->pkg, PKG_STAT_HALFCONFIGURED);
    modstatdb_note(deconpil->pkg);

    /* This means that we *either* go and run postinst abort-deconfigure,
     * *or* queue the package for later configure processing, depending
     * on which error cleanup route gets taken. */
    push_cleanup_fallback(cu_prermdeconfigure, ~ehflag_normaltidy,
                          ok_prermdeconfigure, ehflag_normaltidy,
                          3, (void *)deconpil->pkg, (void *)removing, (void *)pkg);

    if (removing) {
      maintscript_installed(deconpil->pkg, PRERMFILE, "pre-removal",
                            "deconfigure", "in-favour",
                            pkgbin_name(pkg, &pkg->available, pnaw_nonambig),
                            versiondescribe(&pkg->available.version,
                                            vdew_nonambig),
                            "removing",
                            pkg_name(removing, pnaw_nonambig),
                            versiondescribe(&removing->installed.version,
                                            vdew_nonambig),
                            NULL);
    } else {
      maintscript_installed(deconpil->pkg, PRERMFILE, "pre-removal",
                            "deconfigure", "in-favour",
                            pkgbin_name(pkg, &pkg->available, pnaw_nonambig),
                            versiondescribe(&pkg->available.version,
                                            vdew_nonambig),
                            NULL);
    }
  }
}

/**
 * Read the conffiles, and copy the hashes across.
 */
static void
deb_parse_conffiles(const struct pkginfo *pkg, const char *control_conffiles,
                    struct fsys_namenode_queue *newconffiles)
{
  FILE *conff;
  char conffilenamebuf[MAXCONFFILENAME];

  conff = fopen(control_conffiles, "r");
  if (conff == NULL) {
    if (errno == ENOENT)
      return;
    ohshite(_("error trying to open %.250s"), control_conffiles);
  }

  push_cleanup(cu_closestream, ehflag_bombout, 1, conff);

  while (fgets(conffilenamebuf, MAXCONFFILENAME - 2, conff)) {
    struct pkginfo *otherpkg;
    struct fsys_node_pkgs_iter *iter;
    struct fsys_namenode *namenode;
    struct fsys_namenode_list *newconff;
    struct conffile *searchconff;
    char *conffilename = conffilenamebuf;
    char *p;
    enum fsys_namenode_flags confflags = FNNF_NEW_CONFF;

    p = conffilename + strlen(conffilename);
    if (p == conffilename)
      ohshit(_("conffile file contains an empty line"));
    if (p[-1] != '\n')
      ohshit(_("conffile name '%s' is too long, or missing final newline"),
             conffilename);
    p = str_rtrim_spaces(conffilename, p);
    if (p == conffilename)
      continue;

    /* Check for conffile flags. */
    if (conffilename[0] != '/') {
      char *flag = conffilename;
      char *flag_end = strchr(flag, ' ');

      if (flag_end)
        conffilename = flag_end + 1;

      /* If no flag separator is found, assume a missing leading slash. */
      if (flag_end == NULL || (conffilename[0] && conffilename[0] != '/'))
        ohshit(_("conffile name '%s' is not an absolute pathname"), conffilename);

      flag_end[0] = '\0';

      /* Otherwise assume a missing filename after the flag separator. */
      if (conffilename[0] == '\0')
        ohshit(_("conffile name missing after flag '%s'"), flag);

      if (strcmp(flag, "remove-on-upgrade") == 0) {
        confflags |= FNNF_RM_CONFF_ON_UPGRADE;
        confflags &= ~FNNF_NEW_CONFF;
      } else {
        if (c_isspace(flag[0]))
          warning(_("line with conffile filename '%s' has leading white spaces"),
                  conffilename);
        ohshit(_("unknown flag '%s' for conffile '%s'"), flag, conffilename);
      }
    }

    namenode = fsys_hash_find_node(conffilename, FHFF_NONE);
    namenode->oldhash = NEWCONFFILEFLAG;
    newconff = tar_fsys_namenode_queue_push(newconffiles, namenode);

    /*
     * Let's see if any packages have this file.
     *
     * If they do we check to see if they listed it as a conffile,
     * and if they did we copy the hash across. Since (for plain
     * file conffiles, which is the only kind we are supposed to
     * have) there will only be one package which ‘has’ the file,
     * this will usually mean we only look in the package which
     * we are installing now.
     *
     * The ‘conffiles’ data in the status file is ignored when a
     * package is not also listed in the file ownership database as
     * having that file. If several packages are listed as owning
     * the file we pick one at random.
     */
    searchconff = NULL;

    iter = fsys_node_pkgs_iter_new(newconff->namenode);
    while ((otherpkg = fsys_node_pkgs_iter_next(iter))) {
      debug(dbg_conffdetail,
            "process_archive conffile '%s' in package %s - conff ?",
            newconff->namenode->name, pkg_name(otherpkg, pnaw_always));
      for (searchconff = otherpkg->installed.conffiles;
           searchconff && strcmp(newconff->namenode->name, searchconff->name);
           searchconff = searchconff->next)
        debug(dbg_conffdetail,
              "process_archive conffile '%s' in package %s - conff ? not '%s'",
              newconff->namenode->name, pkg_name(otherpkg, pnaw_always),
              searchconff->name);
      if (searchconff) {
        debug(dbg_conff,
              "process_archive conffile '%s' package=%s %s hash=%s",
              newconff->namenode->name, pkg_name(otherpkg, pnaw_always),
              otherpkg == pkg ? "same" : "different!",
              searchconff->hash);
        if (otherpkg == pkg)
          break;
      }
    }
    fsys_node_pkgs_iter_free(iter);

    if (searchconff) {
      /* We don't copy ‘obsolete’; it's not obsolete in the new package. */
      newconff->namenode->oldhash = searchconff->hash;
    } else {
      debug(dbg_conff, "process_archive conffile '%s' no package, no hash",
            newconff->namenode->name);
    }
    newconff->namenode->flags |= confflags;
  }

  if (ferror(conff))
    ohshite(_("read error in %.250s"), control_conffiles);
  pop_cleanup(ehflag_normaltidy); /* conff = fopen() */
  if (fclose(conff))
    ohshite(_("error closing %.250s"), control_conffiles);
}

static struct pkg_queue conflictors = PKG_QUEUE_INIT;

void
enqueue_conflictor(struct pkginfo *pkg)
{
  pkg_queue_push(&conflictors, pkg);
}

static void
pkg_infodb_remove_file(const char *filename, const char *filetype)
{
  if (unlink(filename))
    ohshite(_("unable to delete control info file '%.250s'"), filename);

  debug(dbg_scripts, "removal_bulk info unlinked %s", filename);
}

static struct match_node *match_head = NULL;

static void
pkg_infodb_update_file(const char *filename, const char *filetype)
{
  if (strlen(filetype) > MAXCONTROLFILENAME)
    ohshit(_("old version of package has overly-long info file name starting '%.250s'"),
           filename);

  /* We do the list separately. */
  if (strcmp(filetype, LISTFILE) == 0)
    return;

  /* We keep files to rename in a list as doing the rename immediately
   * might influence the current readdir(), the just renamed file might
   * be returned a second time as it's actually a new file from the
   * point of view of the filesystem. */
  match_head = match_node_new(filename, filetype, match_head);
}

static void
pkg_infodb_update(struct pkginfo *pkg, char *cidir, char *cidirrest)
{
  struct match_node *match_node;
  DIR *dsd;
  struct dirent *de;

  /* Deallocate the match list in case we aborted previously. */
  while ((match_node = match_head)) {
    match_head = match_node->next;
    match_node_free(match_node);
  }

  pkg_infodb_foreach(pkg, &pkg->available, pkg_infodb_update_file);

  while ((match_node = match_head)) {
    strcpy(cidirrest, match_node->filetype);

    if (!rename(cidir, match_node->filename)) {
      debug(dbg_scripts, "process_archive info installed %s as %s",
            cidir, match_node->filename);
    } else if (errno == ENOENT) {
      /* Right, no new version. */
      if (unlink(match_node->filename))
        ohshite(_("unable to remove obsolete info file '%.250s'"),
                match_node->filename);
      debug(dbg_scripts, "process_archive info unlinked %s",
            match_node->filename);
    } else {
      ohshite(_("unable to install (supposed) new info file '%.250s'"), cidir);
    }
    match_head = match_node->next;
    match_node_free(match_node);
  }

  /* The control directory itself. */
  cidirrest[0] = '\0';
  dsd = opendir(cidir);
  if (!dsd)
    ohshite(_("unable to open temp control directory"));
  push_cleanup(cu_closedir, ~0, 1, (void *)dsd);
  while ((de = readdir(dsd))) {
    const char *newinfofilename;

    if (strchr(de->d_name, '.')) {
      debug(dbg_scripts, "process_archive tmp.ci script/file '%s' contains dot",
            de->d_name);
      continue;
    }
    if (strlen(de->d_name) > MAXCONTROLFILENAME)
      ohshit(_("package contains overly-long control info file name (starting '%.50s')"),
             de->d_name);

    strcpy(cidirrest, de->d_name);

    /* First we check it's not a directory. */
    if (rmdir(cidir) == 0)
      ohshit(_("package control info contained directory '%.250s'"), cidir);
    else if (errno != ENOTDIR)
      ohshite(_("package control info rmdir of '%.250s' didn't say not a dir"),
              de->d_name);

    /* Ignore the control file. */
    if (strcmp(de->d_name, CONTROLFILE) == 0) {
      debug(dbg_scripts, "process_archive tmp.ci script/file '%s' is control",
            cidir);
      continue;
    }
    if (strcmp(de->d_name, LISTFILE) == 0) {
      warning(_("package %s contained list as info file"),
              pkgbin_name(pkg, &pkg->available, pnaw_nonambig));
      continue;
    }

    /* Right, install it */
    newinfofilename = pkg_infodb_get_file(pkg, &pkg->available, de->d_name);
    if (rename(cidir, newinfofilename))
      ohshite(_("unable to install new info file '%.250s' as '%.250s'"),
              cidir, newinfofilename);

    debug(dbg_scripts,
          "process_archive tmp.ci script/file '%s' installed as '%s'",
          cidir, newinfofilename);
  }
  pop_cleanup(ehflag_normaltidy); /* closedir */

  /* If the old and new versions use a different infodb layout, get rid
   * of the files using the old layout. */
  if (pkg->installed.multiarch != pkg->available.multiarch &&
      (pkg->installed.multiarch == PKG_MULTIARCH_SAME ||
       pkg->available.multiarch == PKG_MULTIARCH_SAME)) {
    debug(dbg_scripts,
          "process_archive remove old info files after db layout switch");
    pkg_infodb_foreach(pkg, &pkg->installed, pkg_infodb_remove_file);
  }

  dir_sync_path(pkg_infodb_get_dir());
}

static void
pkg_remove_conffile_on_upgrade(struct pkginfo *pkg, struct fsys_namenode *namenode)
{
  struct pkginfo *otherpkg;
  struct fsys_namenode *usenode;
  struct fsys_node_pkgs_iter *iter;
  struct varbuf cdr = VARBUF_INIT;
  struct varbuf cdrext = VARBUF_INIT;
  struct varbuf_state cdrext_state;
  char currenthash[MD5HASHLEN + 1];
  int rc;

  usenode = namenodetouse(namenode, pkg, &pkg->installed);

  rc = conffderef(pkg, &cdr, usenode->name);
  if (rc < 0) {
    debug(dbg_conffdetail, "%s: '%s' conffile dereference error: %s", __func__,
          namenode->name, strerror(errno));
    namenode->oldhash = EMPTYHASHFLAG;
    return;
  }

  varbuf_set_varbuf(&cdrext, &cdr);
  varbuf_snapshot(&cdrext, &cdrext_state);

  iter = fsys_node_pkgs_iter_new(namenode);
  while ((otherpkg = fsys_node_pkgs_iter_next(iter))) {
    debug(dbg_conffdetail, "%s: namenode '%s' owned by other %s?",
          __func__, namenode->name, pkg_name(otherpkg, pnaw_always));

    if (otherpkg == pkg)
      continue;

    debug(dbg_conff, "%s: namenode '%s' owned by other %s, remove-on-upgrade ignored",
         __func__, namenode->name, pkg_name(otherpkg, pnaw_always));
    fsys_node_pkgs_iter_free(iter);
    return;
  }
  fsys_node_pkgs_iter_free(iter);

  /* Remove DPKGDISTEXT variant if still present. */
  varbuf_rollback(&cdrext_state);
  varbuf_add_str(&cdrext, DPKGDISTEXT);

  if (unlink(cdrext.buf) < 0 && errno != ENOENT)
    warning(_("%s: failed to remove '%.250s': %s"),
            pkg_name(pkg, pnaw_nonambig), cdrext.buf,
            strerror(errno));

  md5hash(pkg, currenthash, cdr.buf);

  /* Has it been already removed (e.g. by local admin)? */
  if (strcmp(currenthash, NONEXISTENTFLAG) == 0)
    return;

  /* For unmodified conffiles, we just remove them. */
  if (strcmp(currenthash, namenode->oldhash) == 0) {
    printf(_("Removing obsolete conffile %s ...\n"), cdr.buf);
    if (unlink(cdr.buf) < 0 && errno != ENOENT)
      warning(_("%s: failed to remove '%.250s': %s"),
              pkg_name(pkg, pnaw_nonambig), cdr.buf, strerror(errno));
    return;
  }

  /* Otherwise, preserve the modified conffile. */
  varbuf_rollback(&cdrext_state);
  varbuf_add_str(&cdrext, DPKGOLDEXT);

  printf(_("Obsolete conffile '%s' has been modified by you.\n"), cdr.buf);
  printf(_("Saving as %s ...\n"), cdrext.buf);
  if (rename(cdr.buf, cdrext.buf) < 0)
    warning(_("%s: cannot rename obsolete conffile '%s' to '%s': %s"),
            pkg_name(pkg, pnaw_nonambig),
            cdr.buf, cdrext.buf, strerror(errno));
}

static void
pkg_remove_old_files(struct pkginfo *pkg,
                     struct fsys_namenode_queue *newfiles_queue,
                     struct fsys_namenode_queue *newconffiles)
{
  struct fsys_hash_rev_iter rev_iter;
  struct fsys_namenode_list *cfile;
  struct fsys_namenode *namenode;
  struct stat stab, oldfs;

  /* Before removing any old files, we try to remove obsolete conffiles that
   * have been requested to be removed during upgrade. These conffiles are
   * not tracked as part of the package file lists, so removing them here
   * means we will get their parent directories removed if not present in the
   * new package without needing to do anything special ourselves. */
  for (cfile = newconffiles->head; cfile; cfile = cfile->next) {
    debug(dbg_conffdetail, "%s: removing conffile '%s' for %s?", __func__,
          cfile->namenode->name, pkg_name(pkg, pnaw_always));

    if (!(cfile->namenode->flags & FNNF_RM_CONFF_ON_UPGRADE))
      continue;

    pkg_remove_conffile_on_upgrade(pkg, cfile->namenode);
  }

  fsys_hash_rev_iter_init(&rev_iter, pkg->files);

  while ((namenode = fsys_hash_rev_iter_next(&rev_iter))) {
    struct fsys_namenode *usenode;

    if ((namenode->flags & FNNF_NEW_CONFF) ||
        (namenode->flags & FNNF_RM_CONFF_ON_UPGRADE) ||
        (namenode->flags & FNNF_NEW_INARCHIVE))
      continue;

    usenode = namenodetouse(namenode, pkg, &pkg->installed);

    varbuf_rollback(&fname_state);
    varbuf_add_str(&fnamevb, usenode->name);

    if (!stat(fnamevb.buf, &stab) && S_ISDIR(stab.st_mode)) {
      debug(dbg_eachfiledetail, "process_archive: %s is a directory",
            fnamevb.buf);
      if (dir_is_used_by_others(namenode, pkg))
        continue;
    }

    if (lstat(varbuf_str(&fnamevb), &oldfs)) {
      if (!(errno == ENOENT || errno == ELOOP || errno == ENOTDIR))
        warning(_("could not stat old file '%.250s' so not deleting it: %s"),
                varbuf_str(&fnamevb), strerror(errno));
      continue;
    }
    if (S_ISDIR(oldfs.st_mode)) {
      trig_path_activate(usenode, pkg);

      /* Do not try to remove the root directory. */
      if (strcmp(usenode->name, "/.") == 0)
        continue;

      if (rmdir(varbuf_str(&fnamevb))) {
        warning(_("unable to delete old directory '%.250s': %s"),
                namenode->name, strerror(errno));
      } else if ((namenode->flags & FNNF_OLD_CONFF)) {
        warning(_("old conffile '%.250s' was an empty directory "
                  "(and has now been deleted)"), namenode->name);
      }
    } else {
      struct fsys_namenode_list *sameas = NULL;
      static struct file_ondisk_id empty_ondisk_id;
      struct varbuf cfilename = VARBUF_INIT;

      /*
       * Ok, it's an old file, but is it really not in the new package?
       * It might be known by a different name because of symlinks.
       *
       * We need to check to make sure, so we stat the file, then compare
       * it to the new list. If we find a dev/inode match, we assume they
       * are the same file, and leave it alone. NOTE: we don't check in
       * other packages for sanity reasons (we don't want to stat _all_
       * the files on the system).
       *
       * We run down the list of _new_ files in this package. This keeps
       * the process a little leaner. We are only worried about new ones
       * since ones that stayed the same don't really apply here.
       */

      /* If we can't stat the old or new file, or it's a directory,
       * we leave it up to the normal code. */
      debug(dbg_eachfile, "process_archive: checking %s for same files on "
            "upgrade/downgrade", varbuf_str(&fnamevb));

      for (cfile = newfiles_queue->head; cfile; cfile = cfile->next) {
        /* If the file has been filtered then treat it as if it didn't exist
         * on the file system. */
        if (cfile->namenode->flags & FNNF_FILTERED)
          continue;

        if (cfile->namenode->file_ondisk_id == NULL) {
          struct stat tmp_stat;

          varbuf_set_str(&cfilename, dpkg_fsys_get_dir());
          varbuf_add_str(&cfilename, cfile->namenode->name);

          if (lstat(varbuf_str(&cfilename), &tmp_stat) == 0) {
            struct file_ondisk_id *file_ondisk_id;

            file_ondisk_id = nfmalloc(sizeof(*file_ondisk_id));
            file_ondisk_id->id_dev = tmp_stat.st_dev;
            file_ondisk_id->id_ino = tmp_stat.st_ino;
            cfile->namenode->file_ondisk_id = file_ondisk_id;
          } else {
            if (!(errno == ENOENT || errno == ELOOP || errno == ENOTDIR))
              ohshite(_("unable to stat other new file '%.250s'"),
                      cfile->namenode->name);
            cfile->namenode->file_ondisk_id = &empty_ondisk_id;
            continue;
          }
        }

        if (cfile->namenode->file_ondisk_id == &empty_ondisk_id)
          continue;

        if (oldfs.st_dev == cfile->namenode->file_ondisk_id->id_dev &&
            oldfs.st_ino == cfile->namenode->file_ondisk_id->id_ino) {
          if (sameas)
            warning(_("old file '%.250s' is the same as several new files! "
                      "(both '%.250s' and '%.250s')"), varbuf_str(&fnamevb),
                    sameas->namenode->name, cfile->namenode->name);
          sameas = cfile;
          debug(dbg_eachfile, "process_archive: not removing %s, "
                "since it matches %s", varbuf_str(&fnamevb), cfile->namenode->name);
        }
      }

      varbuf_destroy(&cfilename);

      if ((namenode->flags & FNNF_OLD_CONFF)) {
        if (sameas) {
          if (sameas->namenode->flags & FNNF_NEW_CONFF) {
            if (strcmp(sameas->namenode->oldhash, NEWCONFFILEFLAG) == 0) {
              sameas->namenode->oldhash = namenode->oldhash;
              debug(dbg_eachfile, "process_archive: old conff %s "
                    "is same as new conff %s, copying hash",
                    namenode->name, sameas->namenode->name);
            } else {
              debug(dbg_eachfile, "process_archive: old conff %s "
                    "is same as new conff %s but latter already has hash",
                    namenode->name, sameas->namenode->name);
            }
          }
        } else {
          debug(dbg_eachfile, "process_archive: old conff %s "
                "is disappearing", namenode->name);
          namenode->flags |= FNNF_OBS_CONFF;
          tar_fsys_namenode_queue_push(newconffiles, namenode);
          tar_fsys_namenode_queue_push(newfiles_queue, namenode);
        }
        continue;
      }

      if (sameas)
        continue;

      trig_path_activate(usenode, pkg);

      if (secure_unlink_statted(varbuf_str(&fnamevb), &oldfs)) {
        warning(_("unable to securely remove old file '%.250s': %s"),
                namenode->name, strerror(errno));
      }
    } /* !S_ISDIR */
  }
}

static void
pkg_update_fields(struct pkginfo *pkg, struct fsys_namenode_queue *newconffiles)
{
  struct dependency *newdeplist, **newdeplistlastp;
  struct dependency *dep;
  struct conffile **iconffileslastp, *newiconff;
  struct fsys_namenode_list *cfile;

  /* The dependencies are the most difficult. We have to build
   * a whole new forward dependency tree. At least the reverse
   * links (linking our deppossi's into the reverse chains)
   * can be done by copy_dependency_links. */
  newdeplist = NULL;
  newdeplistlastp = &newdeplist;
  for (dep = pkg->available.depends; dep; dep = dep->next) {
    struct deppossi **newpossilastp, *possi;
    struct dependency *newdep;

    newdep = nfmalloc(sizeof(*newdep));
    newdep->up = pkg;
    newdep->next = NULL;
    newdep->list = NULL;
    newpossilastp = &newdep->list;

    for (possi = dep->list; possi; possi = possi->next) {
      struct deppossi *newpossi;

      newpossi = nfmalloc(sizeof(*newpossi));
      newpossi->up = newdep;
      newpossi->ed = possi->ed;
      newpossi->next = NULL;
      newpossi->rev_next = newpossi->rev_prev = NULL;
      newpossi->arch_is_implicit = possi->arch_is_implicit;
      newpossi->arch = possi->arch;
      newpossi->verrel = possi->verrel;
      if (possi->verrel != DPKG_RELATION_NONE)
        newpossi->version = possi->version;
      else
        dpkg_version_blank(&newpossi->version);
      newpossi->cyclebreak = false;
      *newpossilastp = newpossi;
      newpossilastp = &newpossi->next;
    }
    newdep->type = dep->type;
    *newdeplistlastp = newdep;
    newdeplistlastp = &newdep->next;
  }

  /* Right, now we've replicated the forward tree, we
   * get copy_dependency_links to remove all the old dependency
   * structures from the reverse links and add the new dependency
   * structures in instead. It also copies the new dependency
   * structure pointer for this package into the right field. */
  copy_dependency_links(pkg, &pkg->installed.depends, newdeplist, 0);

  /* We copy the text fields. */
  pkg->installed.essential = pkg->available.essential;
  pkg->installed.is_protected = pkg->available.is_protected;
  pkg->installed.multiarch = pkg->available.multiarch;
  pkg->installed.description = pkg->available.description;
  pkg->installed.maintainer = pkg->available.maintainer;
  pkg->installed.source = pkg->available.source;
  pkg->installed.arch = pkg->available.arch;
  pkg->installed.pkgname_archqual = pkg->available.pkgname_archqual;
  pkg->installed.installedsize = pkg->available.installedsize;
  pkg->installed.version = pkg->available.version;
  pkg->installed.origin = pkg->available.origin;
  pkg->installed.bugs = pkg->available.bugs;

  /* We have to generate our own conffiles structure. */
  pkg->installed.conffiles = NULL;
  iconffileslastp = &pkg->installed.conffiles;
  for (cfile = newconffiles->head; cfile; cfile = cfile->next) {
    newiconff = nfmalloc(sizeof(*newiconff));
    newiconff->next = NULL;
    newiconff->name = nfstrsave(cfile->namenode->name);
    newiconff->hash = nfstrsave(cfile->namenode->oldhash);
    newiconff->flags = CONFFILE_NONE;
    if (cfile->namenode->flags & FNNF_OBS_CONFF)
      newiconff->flags |= CONFFILE_OBSOLETE;
    if (cfile->namenode->flags & FNNF_RM_CONFF_ON_UPGRADE)
      newiconff->flags |= CONFFILE_REMOVE_ON_UPGRADE;
    *iconffileslastp = newiconff;
    iconffileslastp = &newiconff->next;
  }

  /* We can just copy the arbitrary fields list, because it is
   * never even rearranged. Phew! */
  pkg->installed.arbs = pkg->available.arbs;
}

static void
pkg_disappear(struct pkginfo *pkg, struct pkginfo *infavour)
{
  printf(_("(Noting disappearance of %s, which has been completely replaced.)\n"),
         pkg_name(pkg, pnaw_nonambig));
  log_action("disappear", pkg, &pkg->installed);
  debug(dbg_general, "pkg_disappear disappearing %s",
        pkg_name(pkg, pnaw_always));

  trig_activate_packageprocessing(pkg);
  maintscript_installed(pkg, POSTRMFILE,
                        "post-removal script (for disappearance)",
                        "disappear",
                        pkgbin_name(infavour, &infavour->available,
                                    pnaw_nonambig),
                        versiondescribe(&infavour->available.version,
                                        vdew_nonambig),
                        NULL);

  /* OK, now we delete all the stuff in the ‘info’ directory ... */
  debug(dbg_general, "pkg_disappear cleaning info directory");
  pkg_infodb_foreach(pkg, &pkg->installed, pkg_infodb_remove_file);
  dir_sync_path(pkg_infodb_get_dir());

  pkg_set_status(pkg, PKG_STAT_NOTINSTALLED);
  pkg_set_want(pkg, PKG_WANT_UNKNOWN);
  pkg_reset_eflags(pkg);

  dpkg_version_blank(&pkg->configversion);
  pkgbin_blank(&pkg->installed);

  pkg->files_list_valid = false;

  modstatdb_note(pkg);
}

static void
pkg_disappear_others(struct pkginfo *pkg)
{
  struct pkg_hash_iter *iter;
  struct pkginfo *otherpkg;
  struct fsys_namenode_list *cfile;
  struct deppossi *pdep;
  struct dependency *providecheck;
  struct varbuf depprobwhy = VARBUF_INIT;

  iter = pkg_hash_iter_new();
  while ((otherpkg = pkg_hash_iter_next_pkg(iter)) != NULL) {
    ensure_package_clientdata(otherpkg);

    if (otherpkg == pkg ||
        otherpkg->status == PKG_STAT_NOTINSTALLED ||
        otherpkg->status == PKG_STAT_CONFIGFILES ||
        otherpkg->clientdata->istobe == PKG_ISTOBE_REMOVE ||
        !otherpkg->files)
      continue;

    /* Do not try to disappear other packages from the same set
     * if they are Multi-Arch: same */
    if (pkg->installed.multiarch == PKG_MULTIARCH_SAME &&
        otherpkg->installed.multiarch == PKG_MULTIARCH_SAME &&
        otherpkg->set == pkg->set)
      continue;

    debug(dbg_veryverbose, "process_archive checking disappearance %s",
          pkg_name(otherpkg, pnaw_always));

    if (otherpkg->clientdata->istobe != PKG_ISTOBE_NORMAL &&
        otherpkg->clientdata->istobe != PKG_ISTOBE_DECONFIGURE)
      internerr("disappearing package %s is not to be normal or deconfigure, "
                "is to be %d",
                pkg_name(otherpkg, pnaw_always), otherpkg->clientdata->istobe);

    for (cfile = otherpkg->files;
         cfile && strcmp(cfile->namenode->name, "/.") == 0;
         cfile = cfile->next);
    if (!cfile) {
      debug(dbg_stupidlyverbose, "process_archive no non-root, no disappear");
      continue;
    }
    for (cfile = otherpkg->files;
         cfile && !filesavespackage(cfile, otherpkg, pkg);
         cfile = cfile->next);
    if (cfile)
      continue;

    /* So dependency things will give right answers ... */
    otherpkg->clientdata->istobe = PKG_ISTOBE_REMOVE;
    debug(dbg_veryverbose, "process_archive disappear checking dependencies");
    for (pdep = otherpkg->set->depended.installed;
         pdep;
         pdep = pdep->rev_next) {
      if (pdep->up->type != dep_depends &&
          pdep->up->type != dep_predepends &&
          pdep->up->type != dep_recommends)
        continue;

      if (depisok(pdep->up, &depprobwhy, NULL, NULL, false))
        continue;

      debug(dbg_veryverbose,"process_archive cannot disappear: %s",
            varbuf_str(&depprobwhy));
      break;
    }
    if (!pdep) {
      /* If we haven't found a reason not to yet, let's look some more. */
      for (providecheck = otherpkg->installed.depends;
           providecheck;
           providecheck = providecheck->next) {
        if (providecheck->type != dep_provides)
          continue;

        for (pdep = providecheck->list->ed->depended.installed;
             pdep;
             pdep = pdep->rev_next) {
          if (pdep->up->type != dep_depends &&
              pdep->up->type != dep_predepends &&
              pdep->up->type != dep_recommends)
            continue;

          if (depisok(pdep->up, &depprobwhy, NULL, NULL, false))
            continue;

          debug(dbg_veryverbose,
                "process_archive cannot disappear (provides %s): %s",
                providecheck->list->ed->name, varbuf_str(&depprobwhy));
          goto break_from_both_loops_at_once;
        }
      }
      break_from_both_loops_at_once:;
    }
    otherpkg->clientdata->istobe = PKG_ISTOBE_NORMAL;
    if (pdep)
      continue;

    /* No, we're disappearing it. This is the wrong time to go and
     * run maintainer scripts and things, as we can't back out. But
     * what can we do ?  It has to be run this late. */
    pkg_disappear(otherpkg, pkg);
  } /* while (otherpkg= ... */
  pkg_hash_iter_free(iter);
}

/**
 * Check if all instances of a pkgset are getting in sync.
 *
 * If that's the case, the extraction is going to ensure consistency
 * of shared files.
 */
static bool
pkgset_getting_in_sync(struct pkginfo *pkg)
{
  struct pkginfo *otherpkg;

  for (otherpkg = &pkg->set->pkg; otherpkg; otherpkg = otherpkg->arch_next) {
    if (otherpkg == pkg)
      continue;
    if (otherpkg->status <= PKG_STAT_CONFIGFILES)
      continue;
    if (dpkg_version_compare(&pkg->available.version,
                             &otherpkg->installed.version)) {
      return false;
    }
  }

  return true;
}

static void
pkg_remove_files_from_others(struct pkginfo *pkg, struct fsys_namenode_list *newfileslist)
{
  struct fsys_namenode_list *cfile;
  struct pkginfo *otherpkg;

  for (cfile = newfileslist; cfile; cfile = cfile->next) {
    struct fsys_node_pkgs_iter *iter;
    struct pkgset *divpkgset;

    if (!(cfile->namenode->flags & FNNF_ELIDE_OTHER_LISTS))
      continue;

    if (cfile->namenode->divert && cfile->namenode->divert->useinstead) {
      divpkgset = cfile->namenode->divert->pkgset;
      if (divpkgset == pkg->set) {
        debug(dbg_eachfile,
              "process_archive not overwriting any '%s' (overriding, '%s')",
              cfile->namenode->name, cfile->namenode->divert->useinstead->name);
        continue;
      } else {
        debug(dbg_eachfile,
              "process_archive looking for overwriting '%s' (overridden by %s)",
              cfile->namenode->name, divpkgset ? divpkgset->name : "<local>");
      }
    } else {
      divpkgset = NULL;
      debug(dbg_eachfile, "process_archive looking for overwriting '%s'",
            cfile->namenode->name);
    }

    iter = fsys_node_pkgs_iter_new(cfile->namenode);
    while ((otherpkg = fsys_node_pkgs_iter_next(iter))) {
      debug(dbg_eachfiledetail, "process_archive ... found in %s",
            pkg_name(otherpkg, pnaw_always));

      /* A pkgset can share files between instances, so there's no point
       * in rewriting the file that's already in place. */
      if (otherpkg->set == pkg->set)
        continue;

      if (otherpkg->set == divpkgset) {
        debug(dbg_eachfiledetail, "process_archive ... diverted, skipping");
        continue;
      }

      if (cfile->namenode->flags & FNNF_NEW_CONFF)
        conffile_mark_obsolete(otherpkg, cfile->namenode);

      /* If !files_list_valid then it's one of the disappeared packages above
       * or we have already updated the files list file, and we don't bother
       * with it here, clearly. */
      if (!otherpkg->files_list_valid)
        continue;

      /* Found one. We delete the list entry for this file,
       * (and any others in the same package) and then mark the package
       * as requiring a reread. */
      write_filelist_except(otherpkg, &otherpkg->installed,
                            otherpkg->files, FNNF_ELIDE_OTHER_LISTS);
      debug(dbg_veryverbose, "process_archive overwrote from %s",
            pkg_name(otherpkg, pnaw_always));
    }
    fsys_node_pkgs_iter_free(iter);
  }
}

static void
pkg_remove_backup_files(struct pkginfo *pkg, struct fsys_namenode_list *newfileslist)
{
  struct fsys_namenode_list *cfile;

  for (cfile = newfileslist; cfile; cfile = cfile->next) {
    struct fsys_namenode *usenode;

    if (cfile->namenode->flags & FNNF_NEW_CONFF)
      continue;

    usenode = namenodetouse(cfile->namenode, pkg, &pkg->installed);

    /* Do not try to remove backups for the root directory. */
    if (strcmp(usenode->name, "/.") == 0)
      continue;

    varbuf_rollback(&fnametmp_state);
    varbuf_add_str(&fnametmpvb, usenode->name);
    varbuf_add_str(&fnametmpvb, DPKGTEMPEXT);
    path_remove_tree(varbuf_str(&fnametmpvb));
  }
}

void process_archive(const char *filename) {
  static const struct tar_operations tf = {
    .read = tarfileread,
    .extract_file = tarobject,
    .link = tarobject,
    .symlink = tarobject,
    .mkdir = tarobject,
    .mknod = tarobject,
  };

  /* These need to be static so that we can pass their addresses to
   * push_cleanup as arguments to the cu_xxx routines; if an error occurs
   * we unwind the stack before processing the cleanup list, and these
   * variables had better still exist ... */
  static int p1[2];
  static enum pkgstatus oldversionstatus;
  static struct tarcontext tc;

  struct tar_archive tar;
  struct dpkg_error err;
  enum parsedbflags parsedb_flags;
  int rc;
  pid_t pid;
  struct pkginfo *pkg, *otherpkg;
  struct pkg_list *conflictor_iter;
  char *cidir = NULL;
  char *cidirrest;
  char *psize;
  const char *pfilename;
  struct fsys_namenode_queue newconffiles, newfiles_queue;
  struct stat stab;

  cleanup_pkg_failed= cleanup_conflictor_failed= 0;

  pfilename = summarize_filename(filename);

  if (stat(filename, &stab))
    ohshite(_("cannot access archive '%s'"), filename);

  /* We can't ‘tentatively-reassemble’ packages. */
  if (f_act) {
    if (!deb_reassemble(&filename, &pfilename))
      return;
  }

  /* Verify the package. */
  if (f_debsig)
    deb_verify(filename);

  /* Get the control information directory. */
  cidir = get_control_dir(cidir);
  cidirrest = cidir + strlen(cidir);
  push_cleanup(cu_cidir, ~0, 2, (void *)cidir, (void *)cidirrest);

  pid = subproc_fork();
  if (pid == 0) {
    cidirrest[-1] = '\0';
    execlp(BACKEND, BACKEND, "--control", filename, cidir, NULL);
    ohshite(_("unable to execute %s (%s)"),
            _("package control information extraction"), BACKEND);
  }
  subproc_reap(pid, BACKEND " --control", 0);

  /* We want to guarantee the extracted files are on the disk, so that the
   * subsequent renames to the info database do not end up with old or zero
   * length files in case of a system crash. As neither dpkg-deb nor tar do
   * explicit fsync()s, we have to do them here.
   * XXX: This could be avoided by switching to an internal tar extractor. */
  dir_sync_contents(cidir);

  strcpy(cidirrest,CONTROLFILE);

  if (cipaction->arg_int == act_avail)
    parsedb_flags = pdb_parse_available;
  else
    parsedb_flags = pdb_parse_binary;
  parsedb_flags |= pdb_ignore_archives;
  if (in_force(FORCE_BAD_VERSION))
    parsedb_flags |= pdb_lax_version_parser;

  parsedb(cidir, parsedb_flags, &pkg);

  if (!pkg->archives) {
    pkg->archives = nfmalloc(sizeof(*pkg->archives));
    pkg->archives->next = NULL;
    pkg->archives->name = NULL;
    pkg->archives->msdosname = NULL;
    pkg->archives->md5sum = NULL;
  }
  /* Always nfmalloc. Otherwise, we may overwrite some other field (like
   * md5sum). */
  psize = nfmalloc(30);
  sprintf(psize, "%jd", (intmax_t)stab.st_size);
  pkg->archives->size = psize;

  if (cipaction->arg_int == act_avail) {
    printf(_("Recorded info about %s from %s.\n"),
           pkgbin_name(pkg, &pkg->available, pnaw_nonambig), pfilename);
    pop_cleanup(ehflag_normaltidy);
    return;
  }

  if (pkg->available.arch->type != DPKG_ARCH_ALL &&
      pkg->available.arch->type != DPKG_ARCH_NATIVE &&
      pkg->available.arch->type != DPKG_ARCH_FOREIGN)
    forcibleerr(FORCE_ARCHITECTURE,
                _("package architecture (%s) does not match system (%s)"),
                pkg->available.arch->name,
                dpkg_arch_get(DPKG_ARCH_NATIVE)->name);

  clear_deconfigure_queue();
  clear_istobes();

  if (wanttoinstall(pkg)) {
    pkg_set_want(pkg, PKG_WANT_INSTALL);
  } else {
    pop_cleanup(ehflag_normaltidy);
    return;
  }

  /* Deconfigure other instances from a pkgset if they are not in sync. */
  for (otherpkg = &pkg->set->pkg; otherpkg; otherpkg = otherpkg->arch_next) {
    if (otherpkg == pkg)
      continue;
    if (otherpkg->status <= PKG_STAT_HALFCONFIGURED)
      continue;

    if (dpkg_version_compare(&pkg->available.version,
                             &otherpkg->installed.version))
      enqueue_deconfigure(otherpkg, NULL, PKG_WANT_UNKNOWN);
  }

  pkg_check_depcon(pkg, pfilename);

  ensure_allinstfiles_available();
  fsys_hash_init();
  trig_file_interests_ensure();

  printf(_("Preparing to unpack %s ...\n"), pfilename);

  if (pkg->status != PKG_STAT_NOTINSTALLED &&
      pkg->status != PKG_STAT_CONFIGFILES) {
    log_action("upgrade", pkg, &pkg->installed);
  } else {
    log_action("install", pkg, &pkg->available);
  }

  if (!f_act) {
    pop_cleanup(ehflag_normaltidy);
    return;
  }

  /*
   * OK, we're going ahead.
   */

  trig_activate_packageprocessing(pkg);
  strcpy(cidirrest, TRIGGERSCIFILE);
  trig_parse_ci(cidir, NULL, trig_cicb_statuschange_activate, pkg, &pkg->available);

  /* Read the conffiles, and copy the hashes across. */
  newconffiles.head = NULL;
  newconffiles.tail = &newconffiles.head;
  push_cleanup(cu_fileslist, ~0, 0);
  strcpy(cidirrest,CONFFILESFILE);
  deb_parse_conffiles(pkg, cidir, &newconffiles);

  /* All the old conffiles are marked with a flag, so that we don't delete
   * them if they seem to disappear completely. */
  pkg_conffiles_mark_old(pkg);
  for (conflictor_iter = conflictors.head;
       conflictor_iter;
       conflictor_iter = conflictor_iter->next)
    pkg_conffiles_mark_old(conflictor_iter->pkg);

  oldversionstatus= pkg->status;

  if (oldversionstatus > PKG_STAT_INSTALLED)
    internerr("package %s state %d is out-of-bounds",
              pkg_name(pkg, pnaw_always), oldversionstatus);

  debug(dbg_general,"process_archive oldversionstatus=%s",
        statusstrings[oldversionstatus]);

  if (oldversionstatus == PKG_STAT_HALFCONFIGURED ||
      oldversionstatus == PKG_STAT_TRIGGERSAWAITED ||
      oldversionstatus == PKG_STAT_TRIGGERSPENDING ||
      oldversionstatus == PKG_STAT_INSTALLED) {
    pkg_set_eflags(pkg, PKG_EFLAG_REINSTREQ);
    pkg_set_status(pkg, PKG_STAT_HALFCONFIGURED);
    modstatdb_note(pkg);
    push_cleanup(cu_prermupgrade, ~ehflag_normaltidy, 1, (void *)pkg);
    maintscript_fallback(pkg, PRERMFILE, "pre-removal", cidir, cidirrest,
                         "upgrade", "failed-upgrade");
    pkg_set_status(pkg, PKG_STAT_UNPACKED);
    oldversionstatus = PKG_STAT_UNPACKED;
    modstatdb_note(pkg);
  }

  pkg_deconfigure_others(pkg);

  for (conflictor_iter = conflictors.head;
       conflictor_iter;
       conflictor_iter = conflictor_iter->next) {
    struct pkginfo *conflictor = conflictor_iter->pkg;

    if (!(conflictor->status == PKG_STAT_HALFCONFIGURED ||
          conflictor->status == PKG_STAT_TRIGGERSAWAITED ||
          conflictor->status == PKG_STAT_TRIGGERSPENDING ||
          conflictor->status == PKG_STAT_INSTALLED))
      continue;

    trig_activate_packageprocessing(conflictor);
    pkg_set_status(conflictor, PKG_STAT_HALFCONFIGURED);
    modstatdb_note(conflictor);
    push_cleanup(cu_prerminfavour, ~ehflag_normaltidy,
                 2, conflictor, pkg);
    maintscript_installed(conflictor, PRERMFILE, "pre-removal",
                          "remove", "in-favour",
                          pkgbin_name(pkg, &pkg->available, pnaw_nonambig),
                          versiondescribe(&pkg->available.version,
                                          vdew_nonambig),
                          NULL);
    pkg_set_status(conflictor, PKG_STAT_HALFINSTALLED);
    modstatdb_note(conflictor);
  }

  pkg_set_eflags(pkg, PKG_EFLAG_REINSTREQ);
  if (pkg->status == PKG_STAT_NOTINSTALLED) {
    pkg->installed.version= pkg->available.version;
    pkg->installed.multiarch = pkg->available.multiarch;
  }
  pkg_set_status(pkg, PKG_STAT_HALFINSTALLED);
  modstatdb_note(pkg);
  if (oldversionstatus == PKG_STAT_NOTINSTALLED) {
    push_cleanup(cu_preinstverynew, ~ehflag_normaltidy,
                 3,(void*)pkg,(void*)cidir,(void*)cidirrest);
    maintscript_new(pkg, PREINSTFILE, "pre-installation", cidir, cidirrest,
                    "install", NULL);
  } else if (oldversionstatus == PKG_STAT_CONFIGFILES) {
    push_cleanup(cu_preinstnew, ~ehflag_normaltidy,
                 3,(void*)pkg,(void*)cidir,(void*)cidirrest);
    maintscript_new(pkg, PREINSTFILE, "pre-installation", cidir, cidirrest,
                    "install",
                    versiondescribe(&pkg->installed.version, vdew_nonambig),
                    versiondescribe(&pkg->available.version, vdew_nonambig),
                    NULL);
  } else {
    push_cleanup(cu_preinstupgrade, ~ehflag_normaltidy,
                 4,(void*)pkg,(void*)cidir,(void*)cidirrest,(void*)&oldversionstatus);
    maintscript_new(pkg, PREINSTFILE, "pre-installation", cidir, cidirrest,
                    "upgrade",
                    versiondescribe(&pkg->installed.version, vdew_nonambig),
                    versiondescribe(&pkg->available.version, vdew_nonambig),
                    NULL);
  }

  if (oldversionstatus == PKG_STAT_NOTINSTALLED ||
      oldversionstatus == PKG_STAT_CONFIGFILES) {
    printf(_("Unpacking %s (%s) ...\n"),
           pkgbin_name(pkg, &pkg->available, pnaw_nonambig),
           versiondescribe(&pkg->available.version, vdew_nonambig));
  } else {
    printf(_("Unpacking %s (%s) over (%s) ...\n"),
           pkgbin_name(pkg, &pkg->available, pnaw_nonambig),
           versiondescribe(&pkg->available.version, vdew_nonambig),
           versiondescribe(&pkg->installed.version, vdew_nonambig));
  }

  /*
   * Now we unpack the archive, backing things up as we go.
   * For each file, we check to see if it already exists.
   * There are several possibilities:
   *
   * + We are trying to install a non-directory ...
   *  - It doesn't exist. In this case we simply extract it.
   *  - It is a plain file, device, symlink, &c. We do an ‘atomic
   *    overwrite’ using link() and rename(), but leave a backup copy.
   *    Later, when we delete the backup, we remove it from any other
   *    packages' lists.
   *  - It is a directory. In this case it depends on whether we're
   *    trying to install a symlink or something else.
   *   = If we're not trying to install a symlink we move the directory
   *     aside and extract the node. Later, when we recursively remove
   *     the backed-up directory, we remove it from any other packages'
   *     lists.
   *   = If we are trying to install a symlink we do nothing - ie,
   *     dpkg will never replace a directory tree with a symlink. This
   *     is to avoid embarrassing effects such as replacing a directory
   *     tree with a link to a link to the original directory tree.
   * + We are trying to install a directory ...
   *  - It doesn't exist. We create it with the appropriate modes.
   *  - It exists as a directory or a symlink to one. We do nothing.
   *  - It is a plain file or a symlink (other than to a directory).
   *    We move it aside and create the directory. Later, when we
   *    delete the backup, we remove it from any other packages' lists.
   *
   *                   Install non-dir   Install symlink   Install dir
   *  Exists not               X               X                X
   *  File/node/symlink       LXR             LXR              BXR
   *  Directory               BXR              -                -
   *
   *    X: extract file/node/link/directory
   *   LX: atomic overwrite leaving backup
   *    B: ordinary backup
   *    R: later remove from other packages' lists
   *    -: do nothing
   *
   * After we've done this we go through the remaining things in the
   * lists of packages we're trying to remove (including the old
   * version of the current package). This happens in reverse order,
   * so that we process files before the directories (or symlinks-to-
   * directories) containing them.
   *
   * + If the thing is a conffile then we leave it alone for the purge
   *   operation.
   * + Otherwise, there are several possibilities too:
   *  - The listed thing does not exist. We ignore it.
   *  - The listed thing is a directory or a symlink to a directory.
   *    We delete it only if it isn't listed in any other package.
   *  - The listed thing is not a directory, but was part of the package
   *    that was upgraded, we check to make sure the files aren't the
   *    same ones from the old package by checking dev/inode
   *  - The listed thing is not a directory or a symlink to one (ie,
   *    it's a plain file, device, pipe, &c, or a symlink to one, or a
   *    dangling symlink). We delete it.
   *
   * The removed packages' list becomes empty (of course, the new
   * version of the package we're installing will have a new list,
   * which replaces the old version's list).
   *
   * If at any stage we remove a file from a package's list, and the
   * package isn't one we're already processing, and the package's
   * list becomes empty as a result, we ‘vanish’ the package. This
   * means that we run its postrm with the ‘disappear’ argument, and
   * put the package in the ‘not-installed’ state. If it had any
   * conffiles, their hashes and ownership will have been transferred
   * already, so we just ignore those and forget about them from the
   * point of view of the disappearing package.
   *
   * NOTE THAT THE OLD POSTRM IS RUN AFTER THE NEW PREINST, since the
   * files get replaced ‘as we go’.
   */

  m_pipe(p1);
  push_cleanup(cu_closepipe, ehflag_bombout, 1, (void *)&p1[0]);
  pid = subproc_fork();
  if (pid == 0) {
    m_dup2(p1[1],1); close(p1[0]); close(p1[1]);
    execlp(BACKEND, BACKEND, "--fsys-tarfile", filename, NULL);
    ohshite(_("unable to execute %s (%s)"),
            _("package filesystem archive extraction"), BACKEND);
  }
  close(p1[1]);
  p1[1] = -1;

  newfiles_queue.head = NULL;
  newfiles_queue.tail = &newfiles_queue.head;
  tc.newfiles_queue = &newfiles_queue;
  push_cleanup(cu_fileslist, ~0, 0);
  tc.pkg= pkg;
  tc.backendpipe= p1[0];
  tc.pkgset_getting_in_sync = pkgset_getting_in_sync(pkg);

  /* Setup the tar archive. */
  tar.err = DPKG_ERROR_OBJECT;
  tar.ctx = &tc;
  tar.ops = &tf;

  rc = tar_extractor(&tar);
  if (rc)
    dpkg_error_print(&tar.err,
                     _("corrupted filesystem tarfile in package archive"));
  if (fd_skip(p1[0], -1, &err) < 0)
    ohshit(_("cannot zap possible trailing zeros from dpkg-deb: %s"), err.str);
  close(p1[0]);
  p1[0] = -1;
  subproc_reap(pid, BACKEND " --fsys-tarfile", SUBPROC_NOPIPE);

  tar_deferred_extract(newfiles_queue.head, pkg);

  if (oldversionstatus == PKG_STAT_HALFINSTALLED ||
      oldversionstatus == PKG_STAT_UNPACKED) {
    /* Packages that were in ‘installed’ and ‘postinstfailed’ have been
     * reduced to ‘unpacked’ by now, by the running of the prerm script. */
    pkg_set_status(pkg, PKG_STAT_HALFINSTALLED);
    modstatdb_note(pkg);
    push_cleanup(cu_postrmupgrade, ~ehflag_normaltidy, 1, (void *)pkg);
    maintscript_fallback(pkg, POSTRMFILE, "post-removal", cidir, cidirrest,
                         "upgrade", "failed-upgrade");
  }

  /* If anything goes wrong while tidying up it's a bit late to do
   * anything about it. However, we don't install the new status
   * info yet, so that a future dpkg installation will put everything
   * right (we hope).
   *
   * If something does go wrong later the ‘conflictor’ package will be
   * left in the ‘removal_failed’ state. Removing or installing it
   * will be impossible if it was required because of the conflict with
   * the package we're installing now and (presumably) the dependency
   * by other packages. This means that the files it contains in
   * common with this package will hang around until we successfully
   * get this package installed, after which point we can trust the
   * conflicting package's file list, which will have been updated to
   * remove any files in this package. */
  push_checkpoint(~ehflag_bombout, ehflag_normaltidy);

  /* Now we delete all the files that were in the old version of
   * the package only, except (old or new) conffiles, which we leave
   * alone. */
  pkg_remove_old_files(pkg, &newfiles_queue, &newconffiles);

  /* OK, now we can write the updated files-in-this package list,
   * since we've done away (hopefully) with all the old junk. */
  write_filelist_except(pkg, &pkg->available, newfiles_queue.head, 0);

  /* Trigger interests may have changed.
   * Firstly we go through the old list of interests deleting them.
   * Then we go through the new list adding them. */
  strcpy(cidirrest, TRIGGERSCIFILE);
  trig_parse_ci(pkg_infodb_get_file(pkg, &pkg->installed, TRIGGERSCIFILE),
                trig_cicb_interest_delete, NULL, pkg, &pkg->installed);
  trig_parse_ci(cidir, trig_cicb_interest_add, NULL, pkg, &pkg->available);
  trig_file_interests_save();

  /* We also install the new maintainer scripts, and any other
   * cruft that may have come along with the package. First
   * we go through the existing scripts replacing or removing
   * them as appropriate; then we go through the new scripts
   * (any that are left) and install them. */
  debug(dbg_general, "process_archive updating info directory");
  pkg_infodb_update(pkg, cidir, cidirrest);

  /* We store now the checksums dynamically computed while unpacking. */
  write_filehash_except(pkg, &pkg->available, newfiles_queue.head, 0);

  /*
   * Update the status database.
   *
   * This involves copying each field across from the ‘available’
   * to the ‘installed’ half of the pkg structure.
   * For some of the fields we have to do a complicated construction
   * operation; for others we can just copy the value.
   * We tackle the fields in the order they appear, so that
   * we don't miss any out :-).
   * At least we don't have to copy any strings that are referred
   * to, because these are never modified and never freed.
   */
  pkg_update_fields(pkg, &newconffiles);

  /* In case this was an architecture cross-grade, the in-core pkgset might
   * be in an inconsistent state, with two pkginfo entries having the same
   * architecture, so let's fix that. Note that this does not lose data,
   * as the pkg.available member parsed from the binary should replace the
   * to be cleared duplicate in the other instance. */
  for (otherpkg = &pkg->set->pkg; otherpkg; otherpkg = otherpkg->arch_next) {
    if (otherpkg == pkg)
      continue;
    if (otherpkg->installed.arch != pkg->installed.arch)
      continue;

    if (otherpkg->status != PKG_STAT_NOTINSTALLED)
      internerr("other package %s instance in state %s instead of not-installed",
                pkg_name(otherpkg, pnaw_always), pkg_status_name(otherpkg));

    pkg_blank(otherpkg);
  }

  /* Check for disappearing packages:
   * We go through all the packages on the system looking for ones
   * whose files are entirely part of the one we've just unpacked
   * (and which actually *have* some files!).
   *
   * Any that we find are removed - we run the postrm with ‘disappear’
   * as an argument, and remove their info/... files and status info.
   * Conffiles are ignored (the new package had better do something
   * with them!). */
  pkg_disappear_others(pkg);

  /* Delete files from any other packages' lists.
   * We have to do this before we claim this package is in any
   * sane kind of state, as otherwise we might delete by mistake
   * a file that we overwrote, when we remove the package which
   * had the version we overwrote. To prevent this we make
   * sure that we don't claim this package is OK until we
   * have claimed ‘ownership’ of all its files. */
  pkg_remove_files_from_others(pkg, newfiles_queue.head);

  /* Right, the package we've unpacked is now in a reasonable state.
   * The only thing that we have left to do with it is remove
   * backup files, and we can leave the user to fix that if and when
   * it happens (we leave the reinstall required flag, of course). */
  pkg_set_status(pkg, PKG_STAT_UNPACKED);
  modstatdb_note(pkg);

  /* Now we delete all the backup files that we made when
   * extracting the archive - except for files listed as conffiles
   * in the new package.
   * This time we count it as an error if something goes wrong.
   *
   * Note that we don't ever delete things that were in the old
   * package as a conffile and don't appear at all in the new.
   * They stay recorded as obsolete conffiles and will eventually
   * (if not taken over by another package) be forgotten. */
  pkg_remove_backup_files(pkg, newfiles_queue.head);

  /* OK, we're now fully done with the main package.
   * This is quite a nice state, so we don't unwind past here. */
  pkg_reset_eflags(pkg);
  modstatdb_note(pkg);
  push_checkpoint(~ehflag_bombout, ehflag_normaltidy);

  /* Only the removal of the conflictor left to do.
   * The files list for the conflictor is still a little inconsistent in-core,
   * as we have not yet updated the filename->packages mappings; however,
   * the package->filenames mapping is. */
  while (!pkg_queue_is_empty(&conflictors)) {
    struct pkginfo *conflictor = pkg_queue_pop(&conflictors);

    /* We need to have the most up-to-date info about which files are
     * what ... */
    ensure_allinstfiles_available();
    printf(_("Removing %s (%s), to allow configuration of %s (%s) ...\n"),
           pkg_name(conflictor, pnaw_nonambig),
           versiondescribe(&conflictor->installed.version, vdew_nonambig),
           pkg_name(pkg, pnaw_nonambig),
           versiondescribe(&pkg->installed.version, vdew_nonambig));
    removal_bulk(conflictor);
  }

  if (cipaction->arg_int == act_install)
    enqueue_package_mark_seen(pkg);
}
