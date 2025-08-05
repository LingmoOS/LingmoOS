/*
 * dpkg - main program for package management
 * cleanup.c - cleanup functions, used when we need to unwind
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
#include <time.h>
#include <utime.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg.h>
#include <dpkg/path.h>
#include <dpkg/options.h>
#include <dpkg/db-fsys.h>

#include "main.h"
#include "archives.h"

int cleanup_pkg_failed=0, cleanup_conflictor_failed=0;

/**
 * Something went wrong and we're undoing.
 *
 * We have the following possible situations for non-conffiles:
 *   «pathname».dpkg-tmp exists - in this case we want to remove
 *    «pathname» if it exists and replace it with «pathname».dpkg-tmp.
 *    This undoes the backup operation.
 *   «pathname».dpkg-tmp does not exist - «pathname» may be on the disk,
 *    as a new file which didn't fail, remove it if it is.
 *
 * In both cases, we also make sure we delete «pathname».dpkg-new in
 * case that's still hanging around.
 *
 * For conffiles, we simply delete «pathname».dpkg-new. For these,
 * «pathname».dpkg-tmp shouldn't exist, as we don't make a backup
 * at this stage. Just to be on the safe side, though, we don't
 * look for it.
 */
void cu_installnew(int argc, void **argv) {
  struct fsys_namenode *namenode = argv[0];
  struct stat stab;

  cleanup_pkg_failed++; cleanup_conflictor_failed++;

  debug(dbg_eachfile, "cu_installnew '%s' flags=%o",
        namenode->name, namenode->flags);

  setupfnamevbs(namenode->name);

  if (!(namenode->flags & FNNF_NEW_CONFF) && !lstat(fnametmpvb.buf,&stab)) {
    /* OK, «pathname».dpkg-tmp exists. Remove «pathname» and
     * restore «pathname».dpkg-tmp ... */
    if (namenode->flags & FNNF_NO_ATOMIC_OVERWRITE) {
      /* If we can't do an atomic overwrite we have to delete first any
       * link to the new version we may have created. */
      debug(dbg_eachfiledetail,"cu_installnew restoring nonatomic");
      if (secure_remove(fnamevb.buf) && errno != ENOENT && errno != ENOTDIR)
        ohshite(_("unable to remove newly-installed version of '%.250s' to allow"
                " reinstallation of backup copy"),namenode->name);
    } else {
      debug(dbg_eachfiledetail,"cu_installnew restoring atomic");
    }
    /* Either we can do an atomic restore, or we've made room: */
    if (rename(fnametmpvb.buf,fnamevb.buf))
      ohshite(_("unable to restore backup version of '%.250s'"), namenode->name);
    /* If «pathname».dpkg-tmp was still a hard link to «pathname», then the
     * atomic rename did nothing, so we make sure to remove the backup. */
    else if (unlink(fnametmpvb.buf) && errno != ENOENT)
      ohshite(_("unable to remove backup copy of '%.250s'"), namenode->name);
  } else if (namenode->flags & FNNF_PLACED_ON_DISK) {
    debug(dbg_eachfiledetail,"cu_installnew removing new file");
    if (secure_remove(fnamevb.buf) && errno != ENOENT && errno != ENOTDIR)
      ohshite(_("unable to remove newly-installed version of '%.250s'"),
	      namenode->name);
  } else {
    debug(dbg_eachfiledetail,"cu_installnew not restoring");
  }
  /* Whatever, we delete «pathname».dpkg-new now, if it still exists. */
  if (secure_remove(fnamenewvb.buf) && errno != ENOENT && errno != ENOTDIR)
    ohshite(_("unable to remove newly-extracted version of '%.250s'"),
            namenode->name);

  cleanup_pkg_failed--; cleanup_conflictor_failed--;
}

void cu_prermupgrade(int argc, void **argv) {
  struct pkginfo *pkg= (struct pkginfo*)argv[0];

  if (cleanup_pkg_failed++) return;
  maintscript_postinst(pkg, "abort-upgrade",
                       versiondescribe(&pkg->available.version, vdew_nonambig),
                       NULL);
  pkg_clear_eflags(pkg, PKG_EFLAG_REINSTREQ);
  post_postinst_tasks(pkg, PKG_STAT_INSTALLED);
  cleanup_pkg_failed--;
}

/*
 * Also has conflictor in argv[1] and infavour in argv[2].
 * conflictor may be NULL if deconfigure was due to Breaks or multi-arch
 * instance sync.
 */
void ok_prermdeconfigure(int argc, void **argv) {
  struct pkginfo *deconf= (struct pkginfo*)argv[0];

  if (cipaction->arg_int == act_install)
    enqueue_package(deconf);
}

/*
 * conflictor may be NULL if deconfigure was due to Breaks or multi-arch
 * instance sync.
 */
void cu_prermdeconfigure(int argc, void **argv) {
  struct pkginfo *deconf= (struct pkginfo*)argv[0];
  struct pkginfo *conflictor = (struct pkginfo *)argv[1];
  struct pkginfo *infavour= (struct pkginfo*)argv[2];

  if (conflictor) {
    maintscript_postinst(deconf, "abort-deconfigure",
                         "in-favour",
                         pkgbin_name(infavour, &infavour->available,
                                     pnaw_nonambig),
                         versiondescribe(&infavour->available.version,
                                         vdew_nonambig),
                         "removing",
                         pkg_name(conflictor, pnaw_nonambig),
                         versiondescribe(&conflictor->installed.version,
                                         vdew_nonambig),
                         NULL);
  } else {
    maintscript_postinst(deconf, "abort-deconfigure",
                         "in-favour",
                         pkgbin_name(infavour, &infavour->available,
                                     pnaw_nonambig),
                         versiondescribe(&infavour->available.version,
                                         vdew_nonambig),
                         NULL);
  }

  post_postinst_tasks(deconf, PKG_STAT_INSTALLED);
}

void cu_prerminfavour(int argc, void **argv) {
  struct pkginfo *conflictor= (struct pkginfo*)argv[0];
  struct pkginfo *infavour= (struct pkginfo*)argv[1];

  if (cleanup_conflictor_failed++) return;
  maintscript_postinst(conflictor, "abort-remove",
                       "in-favour",
                       pkgbin_name(infavour, &infavour->available,
                                   pnaw_nonambig),
                       versiondescribe(&infavour->available.version,
                                       vdew_nonambig),
                       NULL);
  pkg_clear_eflags(conflictor, PKG_EFLAG_REINSTREQ);
  post_postinst_tasks(conflictor, PKG_STAT_INSTALLED);
  cleanup_conflictor_failed--;
}

void cu_preinstverynew(int argc, void **argv) {
  struct pkginfo *pkg= (struct pkginfo*)argv[0];
  char *cidir= (char*)argv[1];
  char *cidirrest= (char*)argv[2];

  if (cleanup_pkg_failed++) return;
  maintscript_new(pkg, POSTRMFILE, "post-removal", cidir, cidirrest,
                  "abort-install", NULL);
  pkg_set_status(pkg, PKG_STAT_NOTINSTALLED);
  pkg_clear_eflags(pkg, PKG_EFLAG_REINSTREQ);
  pkgbin_blank(&pkg->installed);
  modstatdb_note(pkg);
  cleanup_pkg_failed--;
}

void cu_preinstnew(int argc, void **argv) {
  struct pkginfo *pkg= (struct pkginfo*)argv[0];
  char *cidir= (char*)argv[1];
  char *cidirrest= (char*)argv[2];

  if (cleanup_pkg_failed++) return;
  maintscript_new(pkg, POSTRMFILE, "post-removal", cidir, cidirrest,
                  "abort-install",
                  versiondescribe(&pkg->installed.version, vdew_nonambig),
                  versiondescribe(&pkg->available.version, vdew_nonambig),
                  NULL);
  pkg_set_status(pkg, PKG_STAT_CONFIGFILES);
  pkg_clear_eflags(pkg, PKG_EFLAG_REINSTREQ);
  modstatdb_note(pkg);
  cleanup_pkg_failed--;
}

void cu_preinstupgrade(int argc, void **argv) {
  struct pkginfo *pkg= (struct pkginfo*)argv[0];
  char *cidir= (char*)argv[1];
  char *cidirrest= (char*)argv[2];
  enum pkgstatus *oldstatusp= (enum pkgstatus*)argv[3];

  if (cleanup_pkg_failed++) return;
  maintscript_new(pkg, POSTRMFILE, "post-removal", cidir, cidirrest,
                  "abort-upgrade",
                  versiondescribe(&pkg->installed.version, vdew_nonambig),
                  versiondescribe(&pkg->available.version, vdew_nonambig),
                  NULL);
  pkg_set_status(pkg, *oldstatusp);
  pkg_clear_eflags(pkg, PKG_EFLAG_REINSTREQ);
  modstatdb_note(pkg);
  cleanup_pkg_failed--;
}

void cu_postrmupgrade(int argc, void **argv) {
  struct pkginfo *pkg= (struct pkginfo*)argv[0];

  if (cleanup_pkg_failed++) return;
  maintscript_installed(pkg, PREINSTFILE, "pre-installation",
                        "abort-upgrade",
                        versiondescribe(&pkg->available.version, vdew_nonambig),
                        NULL);
  cleanup_pkg_failed--;
}

void cu_prermremove(int argc, void **argv) {
  struct pkginfo *pkg= (struct pkginfo*)argv[0];
  const enum pkgstatus *oldpkgstatus = (enum pkgstatus *)argv[1];

  if (cleanup_pkg_failed++) return;
  maintscript_postinst(pkg, "abort-remove", NULL);
  pkg_clear_eflags(pkg, PKG_EFLAG_REINSTREQ);
  post_postinst_tasks(pkg, *oldpkgstatus);
  cleanup_pkg_failed--;
}
