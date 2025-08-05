/*
 * libdpkg - Debian packaging suite library routines
 * dbmodify.c - routines for managing dpkg database updates
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2001 Wichert Akkerman <wichert@debian.org>
 * Copyright © 2006-2014 Guillem Jover <guillem@debian.org>
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
#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/file.h>
#include <dpkg/dir.h>
#include <dpkg/triglib.h>

static bool db_initialized;

static enum modstatdb_rw cstatus=-1, cflags=0;
static char *lockfile;
static char *frontendlockfile;
static char *statusfile, *availablefile;
static char *importanttmpfile=NULL;
static FILE *importanttmp;
static int nextupdate;
static char *updatesdir;
static int updateslength;
static struct varbuf updatefn;
static struct varbuf_state updatefn_state;
static struct varbuf uvb;

static int ulist_select(const struct dirent *de) {
  const char *p;
  int l;
  for (p= de->d_name, l=0; *p; p++, l++)
    if (!c_isdigit(*p))
      return 0;
  if (l > IMPORTANTMAXLEN)
    ohshit(_("updates directory contains file '%.250s' whose name is too long "
           "(length=%d, max=%d)"), de->d_name, l, IMPORTANTMAXLEN);
  if (updateslength < 0)
    updateslength = l;
  else if (l != updateslength)
    ohshit(_("updates directory contains files with different length names "
           "(both %d and %d)"), l, updateslength);
  return 1;
}

static void cleanupdates(void) {
  struct dirent **cdlist;
  int cdn;

  parsedb(statusfile, pdb_parse_status, NULL);

  updateslength= -1;
  cdn = scandir(updatesdir, &cdlist, &ulist_select, alphasort);
  if (cdn < 0) {
    if (errno == ENOENT) {
      if (cstatus >= msdbrw_write &&
          dir_make_path(updatesdir, 0755) < 0)
        ohshite(_("cannot create the dpkg updates directory %s"),
                updatesdir);
      return;
    }
    ohshite(_("cannot scan updates directory '%.255s'"), updatesdir);
  }

  if (cdn) {
    int i;

    for (i=0; i<cdn; i++) {
      varbuf_rollback(&updatefn_state);
      varbuf_add_str(&updatefn, cdlist[i]->d_name);
      parsedb(updatefn.buf, pdb_parse_update, NULL);
    }

    if (cstatus >= msdbrw_write) {
      writedb(statusfile, wdb_must_sync);

      for (i=0; i<cdn; i++) {
        varbuf_rollback(&updatefn_state);
        varbuf_add_str(&updatefn, cdlist[i]->d_name);
        if (unlink(updatefn.buf))
          ohshite(_("failed to remove incorporated update file %.255s"),
                  updatefn.buf);
      }

      dir_sync_path(updatesdir);
    }

    for (i = 0; i < cdn; i++)
      free(cdlist[i]);
  }
  free(cdlist);

  nextupdate= 0;
}

static void createimptmp(void) {
  int i;

  onerr_abort++;

  importanttmp= fopen(importanttmpfile,"w");
  if (!importanttmp)
    ohshite(_("unable to create '%.255s'"), importanttmpfile);
  setcloexec(fileno(importanttmp),importanttmpfile);
  for (i=0; i<512; i++) fputs("#padding\n",importanttmp);
  if (ferror(importanttmp))
    ohshite(_("unable to fill %.250s with padding"),importanttmpfile);
  if (fflush(importanttmp))
    ohshite(_("unable to flush %.250s after padding"), importanttmpfile);
  if (fseek(importanttmp,0,SEEK_SET))
    ohshite(_("unable to seek to start of %.250s after padding"),
	    importanttmpfile);

  onerr_abort--;
}

static const struct fni {
  const char *suffix;
  char **store;
} fnis[] = {
  {
    .suffix = LOCKFILE,
    .store = &lockfile,
  }, {
    .suffix = FRONTENDLOCKFILE,
    .store = &frontendlockfile,
  }, {
    .suffix = STATUSFILE,
    .store = &statusfile,
  }, {
    .suffix = AVAILFILE,
    .store = &availablefile,
  }, {
    .suffix = UPDATESDIR,
    .store = &updatesdir,
  }, {
    .suffix = UPDATESDIR "/" IMPORTANTTMP,
    .store = &importanttmpfile,
  }, {
    .suffix = NULL,
    .store = NULL,
  }
};

void
modstatdb_init(void)
{
  const struct fni *fnip;

  if (db_initialized)
    return;

  for (fnip = fnis; fnip->suffix; fnip++) {
    free(*fnip->store);
    *fnip->store = dpkg_db_get_path(fnip->suffix);
  }

  varbuf_init(&updatefn, strlen(updatesdir) + 1 + IMPORTANTMAXLEN);
  varbuf_add_dir(&updatefn, updatesdir);
  varbuf_snapshot(&updatefn, &updatefn_state);

  db_initialized = true;
}

void
modstatdb_done(void)
{
  const struct fni *fnip;

  if (!db_initialized)
    return;

  for (fnip = fnis; fnip->suffix; fnip++) {
    free(*fnip->store);
    *fnip->store = NULL;
  }
  varbuf_destroy(&updatefn);

  db_initialized = false;
}

static int dblockfd = -1;
static int frontendlockfd = -1;

bool
modstatdb_is_locked(void)
{
  int lockfd;
  bool locked;

  if (dblockfd < 0) {
    lockfd = open(lockfile, O_RDONLY);
    if (lockfd < 0) {
      if (errno == ENOENT)
        return false;
      ohshite(_("unable to check lock file for dpkg database directory %s"),
              dpkg_db_get_dir());
    }
  } else {
    lockfd = dblockfd;
  }

  locked = file_is_locked(lockfd, lockfile);

  /* We only close the file if there was no lock open, otherwise we would
   * release the existing lock on close. */
  if (dblockfd < 0)
    close(lockfd);

  return locked;
}

bool
modstatdb_can_lock(void)
{
  if (dblockfd >= 0)
    return true;

  if (getenv("DPKG_FRONTEND_LOCKED") == NULL) {
    frontendlockfd = open(frontendlockfile, O_RDWR | O_CREAT | O_TRUNC, 0660);
    if (frontendlockfd < 0) {
      if (errno == EACCES || errno == EPERM)
        return false;
      else
        ohshite(_("unable to open/create dpkg frontend lock for directory %s"),
                dpkg_db_get_dir());
    }
  } else {
    frontendlockfd = -1;
  }

  dblockfd = open(lockfile, O_RDWR | O_CREAT | O_TRUNC, 0660);
  if (dblockfd < 0) {
    if (errno == EACCES || errno == EPERM)
      return false;
    else
      ohshite(_("unable to open/create dpkg database lock file for directory %s"),
              dpkg_db_get_dir());
  }

  return true;
}

void
modstatdb_lock(void)
{
  if (!modstatdb_can_lock())
    ohshit(_("you do not have permission to lock the dpkg database directory %s"),
           dpkg_db_get_dir());

  if (frontendlockfd >= 0)
    file_lock(&frontendlockfd, FILE_LOCK_NOWAIT, frontendlockfile,
              _("dpkg frontend lock"));
  file_lock(&dblockfd, FILE_LOCK_NOWAIT, lockfile,
            _("dpkg database lock"));
}

void
modstatdb_unlock(void)
{
  /* Unlock. */
  pop_cleanup(ehflag_normaltidy);
  if (frontendlockfd >= 0)
    pop_cleanup(ehflag_normaltidy);

  dblockfd = -1;
  frontendlockfd = -1;
}

enum modstatdb_rw
modstatdb_open(enum modstatdb_rw readwritereq)
{
  bool db_can_access = false;

  modstatdb_init();

  cflags = readwritereq & msdbrw_available_mask;
  readwritereq &= ~msdbrw_available_mask;

  switch (readwritereq) {
  case msdbrw_needsuperuser:
  case msdbrw_needsuperuserlockonly:
    if (getuid() || geteuid())
      ohshit(_("requested operation requires superuser privilege"));
    /* Fall through. */
  case msdbrw_write: case msdbrw_writeifposs:
    db_can_access = access(dpkg_db_get_dir(), W_OK) == 0;
    if (!db_can_access && errno == ENOENT) {
      if (dir_make_path(dpkg_db_get_dir(), 0755) == 0)
        db_can_access = true;
      else if (readwritereq >= msdbrw_write)
        ohshite(_("cannot create the dpkg database directory %s"),
                dpkg_db_get_dir());
      else if (errno == EROFS)
        /* If we cannot create the directory on read-only modes on read-only
         * filesystems, make it look like an access error to be skipped. */
        errno = EACCES;
    }

    if (!db_can_access) {
      if (errno != EACCES)
        ohshite(_("unable to access the dpkg database directory %s"),
                dpkg_db_get_dir());
      else if (readwritereq >= msdbrw_write)
        ohshit(_("required read/write access to the dpkg database directory %s"),
               dpkg_db_get_dir());
      cstatus= msdbrw_readonly;
    } else {
      modstatdb_lock();
      cstatus= (readwritereq == msdbrw_needsuperuserlockonly ?
                msdbrw_needsuperuserlockonly :
                msdbrw_write);
    }
    break;
  case msdbrw_readonly:
    cstatus= msdbrw_readonly; break;
  default:
    internerr("unknown modstatdb_rw '%d'", readwritereq);
  }

  dpkg_arch_load_list();

  if (cstatus != msdbrw_needsuperuserlockonly) {
    cleanupdates();
    if (cflags >= msdbrw_available_readonly)
      parsedb(availablefile, pdb_parse_available, NULL);
  }

  if (cstatus >= msdbrw_write) {
    createimptmp();
    varbuf_init(&uvb, 10240);
  }

  trig_fixup_awaiters(cstatus);
  trig_incorporate(cstatus);

  return cstatus;
}

enum modstatdb_rw
modstatdb_get_status(void)
{
  return cstatus;
}

void modstatdb_checkpoint(void) {
  int i;

  if (cstatus < msdbrw_write)
    internerr("modstatdb status '%d' is not writable", cstatus);

  writedb(statusfile, wdb_must_sync);

  for (i=0; i<nextupdate; i++) {
    varbuf_rollback(&updatefn_state);
    varbuf_add_fmt(&updatefn, IMPORTANTFMT, i);

    /* Have we made a real mess? */
    if (varbuf_rollback_len(&updatefn_state) > IMPORTANTMAXLEN)
      internerr("modstatdb update entry name '%s' longer than %d",
                varbuf_rollback_end(&updatefn_state), IMPORTANTMAXLEN);

    if (unlink(updatefn.buf))
      ohshite(_("failed to remove my own update file %.255s"), updatefn.buf);
  }

  dir_sync_path(updatesdir);

  nextupdate= 0;
}

void modstatdb_shutdown(void) {
  if (cflags >= msdbrw_available_write)
    writedb(availablefile, wdb_dump_available);

  switch (cstatus) {
  case msdbrw_write:
    modstatdb_checkpoint();
    /* Tidy up a bit, but don't worry too much about failure. */
    fclose(importanttmp);
    (void)unlink(importanttmpfile);
    varbuf_destroy(&uvb);
    /* Fall through. */
  case msdbrw_needsuperuserlockonly:
    modstatdb_unlock();
  default:
    break;
  }

  pkg_hash_reset();

  modstatdb_done();
}

static void
modstatdb_note_core(struct pkginfo *pkg)
{
  if (cstatus < msdbrw_write)
    internerr("modstatdb status '%d' is not writable", cstatus);

  varbuf_reset(&uvb);
  varbuf_stanza(&uvb, pkg, &pkg->installed);

  if (fwrite(uvb.buf, 1, uvb.used, importanttmp) != uvb.used)
    ohshite(_("unable to write updated status of '%.250s'"),
            pkg_name(pkg, pnaw_nonambig));
  if (fflush(importanttmp))
    ohshite(_("unable to flush updated status of '%.250s'"),
            pkg_name(pkg, pnaw_nonambig));
  if (ftruncate(fileno(importanttmp), uvb.used))
    ohshite(_("unable to truncate for updated status of '%.250s'"),
            pkg_name(pkg, pnaw_nonambig));
  if (fsync(fileno(importanttmp)))
    ohshite(_("unable to fsync updated status of '%.250s'"),
            pkg_name(pkg, pnaw_nonambig));
  if (fclose(importanttmp))
    ohshite(_("unable to close updated status of '%.250s'"),
            pkg_name(pkg, pnaw_nonambig));
  varbuf_rollback(&updatefn_state);
  varbuf_add_fmt(&updatefn, IMPORTANTFMT, nextupdate);
  if (rename(importanttmpfile, updatefn.buf))
    ohshite(_("unable to install updated status of '%.250s'"),
            pkg_name(pkg, pnaw_nonambig));

  dir_sync_path(updatesdir);

  /* Have we made a real mess? */
  if (varbuf_rollback_len(&updatefn_state) > IMPORTANTMAXLEN)
    internerr("modstatdb update entry name '%s' longer than %d",
              varbuf_rollback_end(&updatefn_state), IMPORTANTMAXLEN);

  nextupdate++;

  if (nextupdate > MAXUPDATES) {
    modstatdb_checkpoint();
    nextupdate = 0;
  }

  createimptmp();
}

/*
 * Note: If anyone wants to set some triggers-pending, they must also
 * set status appropriately, or we will undo it. That is, it is legal
 * to call this when pkg->status and pkg->trigpend_head disagree and
 * in that case pkg->status takes precedence and pkg->trigpend_head
 * will be adjusted.
 */
void modstatdb_note(struct pkginfo *pkg) {
  struct trigaw *ta;

  onerr_abort++;

  /* Clear pending triggers here so that only code that sets the status
   * to interesting (for triggers) values has to care about triggers. */
  if (pkg->status != PKG_STAT_TRIGGERSPENDING &&
      pkg->status != PKG_STAT_TRIGGERSAWAITED)
    pkg->trigpend_head = NULL;

  if (pkg->status <= PKG_STAT_CONFIGFILES) {
    for (ta = pkg->trigaw.head; ta; ta = ta->sameaw.next)
      ta->aw = NULL;
    pkg->trigaw.head = pkg->trigaw.tail = NULL;
  }

  if (pkg->status_dirty) {
    log_message("status %s %s %s", pkg_status_name(pkg),
                pkg_name(pkg, pnaw_always),
                versiondescribe_c(&pkg->installed.version, vdew_nonambig));
    statusfd_send("status: %s: %s", pkg_name(pkg, pnaw_nonambig),
                  pkg_status_name(pkg));

    pkg->status_dirty = false;
  }

  if (cstatus >= msdbrw_write)
    modstatdb_note_core(pkg);

  if (!pkg->trigpend_head && pkg->othertrigaw_head) {
    /* Automatically remove us from other packages' Triggers-Awaited.
     * We do this last because we want to maximize our chances of
     * successfully recording the status of the package we were
     * pointed at by our caller, although there is some risk of
     * leaving us in a slightly odd situation which is cleared up
     * by the trigger handling logic in deppossi_ok_found. */
    trig_clear_awaiters(pkg);
  }

  onerr_abort--;
}

void
modstatdb_note_ifwrite(struct pkginfo *pkg)
{
  if (cstatus >= msdbrw_write)
    modstatdb_note(pkg);
}

