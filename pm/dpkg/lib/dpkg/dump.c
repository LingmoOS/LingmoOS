/*
 * libdpkg - Debian packaging suite library routines
 * dump.c - code to write in-core database to a file
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2001 Wichert Akkerman
 * Copyright © 2006,2008-2014 Guillem Jover <guillem@debian.org>
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

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg-array.h>
#include <dpkg/pkg-show.h>
#include <dpkg/string.h>
#include <dpkg/dir.h>
#include <dpkg/parsedump.h>

static inline void
varbuf_add_fieldname(struct varbuf *vb, const struct fieldinfo *fip)
{
  varbuf_add_str(vb, fip->name);
  varbuf_add_str(vb, ": ");
}

void
w_name(struct varbuf *vb,
       const struct pkginfo *pkg, const struct pkgbin *pkgbin,
       enum fwriteflags flags, const struct fieldinfo *fip)
{
  if (pkg->set->name == NULL)
    internerr("pkgset has no name");

  if (flags&fw_printheader)
    varbuf_add_str(vb, "Package: ");
  varbuf_add_str(vb, pkg->set->name);
  if (flags&fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_version(struct varbuf *vb,
          const struct pkginfo *pkg, const struct pkgbin *pkgbin,
          enum fwriteflags flags, const struct fieldinfo *fip)
{
  if (!dpkg_version_is_informative(&pkgbin->version))
    return;
  if (flags&fw_printheader)
    varbuf_add_str(vb, "Version: ");
  varbufversion(vb, &pkgbin->version, vdew_nonambig);
  if (flags&fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_configversion(struct varbuf *vb,
                const struct pkginfo *pkg, const struct pkgbin *pkgbin,
                enum fwriteflags flags, const struct fieldinfo *fip)
{
  if (pkgbin != &pkg->installed)
    return;
  if (!dpkg_version_is_informative(&pkg->configversion))
    return;
  if (pkg->status == PKG_STAT_INSTALLED ||
      pkg->status == PKG_STAT_NOTINSTALLED ||
      pkg->status == PKG_STAT_TRIGGERSPENDING)
    return;
  if (flags&fw_printheader)
    varbuf_add_str(vb, "Config-Version: ");
  varbufversion(vb, &pkg->configversion, vdew_nonambig);
  if (flags&fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_null(struct varbuf *vb,
       const struct pkginfo *pkg, const struct pkgbin *pkgbin,
       enum fwriteflags flags, const struct fieldinfo *fip)
{
}

void
w_section(struct varbuf *vb,
          const struct pkginfo *pkg, const struct pkgbin *pkgbin,
          enum fwriteflags flags, const struct fieldinfo *fip)
{
  const char *value = pkg->section;

  if (str_is_unset(value))
    return;
  if (flags&fw_printheader)
    varbuf_add_str(vb, "Section: ");
  varbuf_add_str(vb, value);
  if (flags&fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_charfield(struct varbuf *vb,
            const struct pkginfo *pkg, const struct pkgbin *pkgbin,
            enum fwriteflags flags, const struct fieldinfo *fip)
{
  const char *value = STRUCTFIELD(pkgbin, fip->integer, const char *);

  if (str_is_unset(value))
    return;
  if (flags & fw_printheader)
    varbuf_add_fieldname(vb, fip);
  varbuf_add_str(vb, value);
  if (flags&fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_archives(struct varbuf *vb,
           const struct pkginfo *pkg, const struct pkgbin *pkgbin,
           enum fwriteflags flags, const struct fieldinfo *fip)
{
  struct archivedetails *archive;

  if (pkgbin != &pkg->available)
    return;
  archive = pkg->archives;
  if (!archive || !STRUCTFIELD(archive, fip->integer, const char *))
    return;

  if (flags&fw_printheader) {
    varbuf_add_str(vb, fip->name);
    varbuf_add_char(vb, ':');
  }

  while (archive) {
    varbuf_add_char(vb, ' ');
    varbuf_add_str(vb, STRUCTFIELD(archive, fip->integer, const char *));
    archive = archive->next;
  }

  if (flags&fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_booleandefno(struct varbuf *vb,
               const struct pkginfo *pkg, const struct pkgbin *pkgbin,
               enum fwriteflags flags, const struct fieldinfo *fip)
{
  bool value = STRUCTFIELD(pkgbin, fip->integer, bool);

  if ((flags & fw_printheader) && !value)
    return;

  if (flags & fw_printheader)
    varbuf_add_fieldname(vb, fip);

  varbuf_add_str(vb, value ? "yes" : "no");

  if (flags & fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_multiarch(struct varbuf *vb,
            const struct pkginfo *pkg, const struct pkgbin *pkgbin,
            enum fwriteflags flags, const struct fieldinfo *fip)
{
  int value = STRUCTFIELD(pkgbin, fip->integer, int);

  if ((flags & fw_printheader) && !value)
    return;

  if (flags & fw_printheader)
    varbuf_add_fieldname(vb, fip);

  varbuf_add_str(vb, multiarchinfos[value].name);

  if (flags & fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_architecture(struct varbuf *vb,
               const struct pkginfo *pkg, const struct pkgbin *pkgbin,
               enum fwriteflags flags, const struct fieldinfo *fip)
{
  if (!pkgbin->arch)
    return;
  if (pkgbin->arch->type == DPKG_ARCH_NONE)
    return;
  if (pkgbin->arch->type == DPKG_ARCH_EMPTY)
    return;

  if (flags & fw_printheader)
    varbuf_add_fieldname(vb, fip);
  varbuf_add_str(vb, pkgbin->arch->name);
  if (flags & fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_priority(struct varbuf *vb,
           const struct pkginfo *pkg, const struct pkgbin *pkgbin,
           enum fwriteflags flags, const struct fieldinfo *fip)
{
  if (pkg->priority == PKG_PRIO_UNKNOWN)
    return;

  if (pkg->priority > PKG_PRIO_UNKNOWN)
    internerr("package %s has out-of-range priority %d",
              pkgbin_name_const(pkg, pkgbin, pnaw_always), pkg->priority);

  if (flags&fw_printheader)
    varbuf_add_str(vb, "Priority: ");
  varbuf_add_str(vb, pkg_priority_name(pkg));
  if (flags&fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_status(struct varbuf *vb,
         const struct pkginfo *pkg, const struct pkgbin *pkgbin,
         enum fwriteflags flags, const struct fieldinfo *fip)
{
  if (pkgbin != &pkg->installed)
    return;

  if (pkg->want > PKG_WANT_PURGE)
    internerr("package %s has unknown want state %d",
              pkgbin_name_const(pkg, pkgbin, pnaw_always), pkg->want);
  if (pkg->eflag > PKG_EFLAG_REINSTREQ)
    internerr("package %s has unknown error state %d",
              pkgbin_name_const(pkg, pkgbin, pnaw_always), pkg->eflag);

  switch (pkg->status) {
  case PKG_STAT_NOTINSTALLED:
  case PKG_STAT_CONFIGFILES:
    if (pkg->trigpend_head || pkg->trigaw.head)
      internerr("package %s in state %s, has awaited or pending triggers",
              pkgbin_name_const(pkg, pkgbin, pnaw_always), pkg_status_name(pkg));
    break;
  case PKG_STAT_HALFINSTALLED:
  case PKG_STAT_UNPACKED:
  case PKG_STAT_HALFCONFIGURED:
    if (pkg->trigpend_head)
      internerr("package %s in state %s, has pending triggers",
              pkgbin_name_const(pkg, pkgbin, pnaw_always), pkg_status_name(pkg));
    break;
  case PKG_STAT_TRIGGERSAWAITED:
    if (pkg->trigaw.head == NULL)
      internerr("package %s in state %s, has no awaited triggers",
                pkgbin_name_const(pkg, pkgbin, pnaw_always), pkg_status_name(pkg));
    break;
  case PKG_STAT_TRIGGERSPENDING:
    if (pkg->trigpend_head == NULL || pkg->trigaw.head)
      internerr("package %s in stata %s, has awaited or no pending triggers",
              pkgbin_name_const(pkg, pkgbin, pnaw_always), pkg_status_name(pkg));
    break;
  case PKG_STAT_INSTALLED:
    if (pkg->trigpend_head || pkg->trigaw.head)
      internerr("package %s in state %s, has awaited or pending triggers",
              pkgbin_name_const(pkg, pkgbin, pnaw_always), pkg_status_name(pkg));
    break;
  default:
    internerr("unknown package status '%d'", pkg->status);
  }

  if (flags&fw_printheader)
    varbuf_add_str(vb, "Status: ");
  varbuf_add_str(vb, pkg_want_name(pkg));
  varbuf_add_char(vb, ' ');
  varbuf_add_str(vb, pkg_eflag_name(pkg));
  varbuf_add_char(vb, ' ');
  varbuf_add_str(vb, pkg_status_name(pkg));
  if (flags&fw_printheader)
    varbuf_add_char(vb, '\n');
}

void varbufdependency(struct varbuf *vb, struct dependency *dep) {
  struct deppossi *dop;
  const char *possdel;

  possdel= "";
  for (dop= dep->list; dop; dop= dop->next) {
    if (dop->up != dep)
      internerr("dependency and deppossi not linked properly");

    varbuf_add_str(vb, possdel);
    possdel = " | ";
    varbuf_add_str(vb, dop->ed->name);
    if (!dop->arch_is_implicit)
      varbuf_add_archqual(vb, dop->arch);
    if (dop->verrel != DPKG_RELATION_NONE) {
      varbuf_add_str(vb, " (");
      switch (dop->verrel) {
      case DPKG_RELATION_EQ:
        varbuf_add_char(vb, '=');
        break;
      case DPKG_RELATION_GE:
        varbuf_add_str(vb, ">=");
        break;
      case DPKG_RELATION_LE:
        varbuf_add_str(vb, "<=");
        break;
      case DPKG_RELATION_GT:
        varbuf_add_str(vb, ">>");
        break;
      case DPKG_RELATION_LT:
        varbuf_add_str(vb, "<<");
        break;
      default:
        internerr("unknown dpkg_relation %d", dop->verrel);
      }
      varbuf_add_char(vb, ' ');
      varbufversion(vb,&dop->version,vdew_nonambig);
      varbuf_add_char(vb, ')');
    }
  }
}

void
w_dependency(struct varbuf *vb,
             const struct pkginfo *pkg, const struct pkgbin *pkgbin,
             enum fwriteflags flags, const struct fieldinfo *fip)
{
  struct dependency *dyp;
  bool dep_found = false;

  for (dyp = pkgbin->depends; dyp; dyp = dyp->next) {
    if (dyp->type != fip->integer) continue;

    if (dyp->up != pkg)
      internerr("dependency and package %s not linked properly",
                pkgbin_name_const(pkg, pkgbin, pnaw_always));

    if (dep_found) {
      varbuf_add_str(vb, ", ");
    } else {
      if (flags & fw_printheader)
        varbuf_add_fieldname(vb, fip);
      dep_found = true;
    }
    varbufdependency(vb,dyp);
  }
  if ((flags & fw_printheader) && dep_found)
    varbuf_add_char(vb, '\n');
}

void
w_conffiles(struct varbuf *vb,
            const struct pkginfo *pkg, const struct pkgbin *pkgbin,
            enum fwriteflags flags, const struct fieldinfo *fip)
{
  struct conffile *i;

  if (!pkgbin->conffiles || pkgbin == &pkg->available)
    return;
  if (flags&fw_printheader)
    varbuf_add_str(vb, "Conffiles:\n");
  for (i = pkgbin->conffiles; i; i = i->next) {
    if (i != pkgbin->conffiles)
      varbuf_add_char(vb, '\n');
    varbuf_add_char(vb, ' ');
    varbuf_add_str(vb, i->name);
    varbuf_add_char(vb, ' ');
    varbuf_add_str(vb, i->hash);
    if (i->flags & CONFFILE_OBSOLETE)
      varbuf_add_str(vb, " obsolete");
    if (i->flags & CONFFILE_REMOVE_ON_UPGRADE)
      varbuf_add_str(vb, " remove-on-upgrade");
  }
  if (flags&fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_trigpend(struct varbuf *vb,
           const struct pkginfo *pkg, const struct pkgbin *pkgbin,
           enum fwriteflags flags, const struct fieldinfo *fip)
{
  struct trigpend *tp;

  if (pkgbin == &pkg->available || !pkg->trigpend_head)
    return;

  if (pkg->status < PKG_STAT_TRIGGERSAWAITED ||
      pkg->status > PKG_STAT_TRIGGERSPENDING)
    internerr("package %s in non-trigger state %s, has pending triggers",
              pkgbin_name_const(pkg, pkgbin, pnaw_always), pkg_status_name(pkg));

  if (flags & fw_printheader)
    varbuf_add_str(vb, "Triggers-Pending:");
  for (tp = pkg->trigpend_head; tp; tp = tp->next) {
    varbuf_add_char(vb, ' ');
    varbuf_add_str(vb, tp->name);
  }
  if (flags & fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
w_trigaw(struct varbuf *vb,
         const struct pkginfo *pkg, const struct pkgbin *pkgbin,
         enum fwriteflags flags, const struct fieldinfo *fip)
{
  struct trigaw *ta;

  if (pkgbin == &pkg->available || !pkg->trigaw.head)
    return;

  if (pkg->status <= PKG_STAT_CONFIGFILES ||
      pkg->status > PKG_STAT_TRIGGERSAWAITED)
    internerr("package %s in state %s, has awaited triggers",
              pkgbin_name_const(pkg, pkgbin, pnaw_always), pkg_status_name(pkg));

  if (flags & fw_printheader)
    varbuf_add_str(vb, "Triggers-Awaited:");
  for (ta = pkg->trigaw.head; ta; ta = ta->sameaw.next) {
    varbuf_add_char(vb, ' ');
    varbuf_add_pkgbin_name(vb, ta->pend, &ta->pend->installed, pnaw_nonambig);
  }
  if (flags & fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
varbuf_add_arbfield(struct varbuf *vb, const struct arbitraryfield *arbfield,
                    enum fwriteflags flags)
{
  if (flags & fw_printheader) {
    varbuf_add_str(vb, arbfield->name);
    varbuf_add_str(vb, ": ");
  }
  varbuf_add_str(vb, arbfield->value);
  if (flags & fw_printheader)
    varbuf_add_char(vb, '\n');
}

void
varbuf_stanza(struct varbuf *vb,
              const struct pkginfo *pkg, const struct pkgbin *pkgbin)
{
  const struct fieldinfo *fip;
  const struct arbitraryfield *afp;

  for (fip= fieldinfos; fip->name; fip++) {
    fip->wcall(vb, pkg, pkgbin, fw_printheader, fip);
  }
  for (afp = pkgbin->arbs; afp; afp = afp->next) {
    varbuf_add_arbfield(vb, afp, fw_printheader);
  }
}

void
write_stanza(FILE *file, const char *filename,
             const struct pkginfo *pkg, const struct pkgbin *pkgbin)
{
  struct varbuf vb = VARBUF_INIT;

  varbuf_stanza(&vb, pkg, pkgbin);

  if (fputs(varbuf_str(&vb), file) < 0)
    ohshite(_("failed to write details of '%.50s' to '%.250s'"),
            pkgbin_name_const(pkg, pkgbin, pnaw_nonambig), filename);

  varbuf_destroy(&vb);
}

void
writedb_stanzas(FILE *fp, const char *filename, enum writedb_flags flags)
{
  static char writebuf[8192];

  struct pkg_array array;
  const char *which;
  struct varbuf vb = VARBUF_INIT;
  int i;

  which = (flags & wdb_dump_available) ? "available" : "status";

  if (setvbuf(fp, writebuf, _IOFBF, sizeof(writebuf)))
    ohshite(_("unable to set buffering on %s database file"), which);

  pkg_array_init_from_hash(&array);
  pkg_array_sort(&array, pkg_sorter_by_nonambig_name_arch);

  for (i = 0; i < array.n_pkgs; i++) {
    struct pkginfo *pkg;
    struct pkgbin *pkgbin;

    pkg = array.pkgs[i];
    pkgbin = (flags & wdb_dump_available) ? &pkg->available : &pkg->installed;

    /* Don't dump stanzas which have no useful content. */
    if (!pkg_is_informative(pkg, pkgbin))
      continue;

    varbuf_stanza(&vb, pkg, pkgbin);
    varbuf_add_char(&vb, '\n');
    if (fputs(varbuf_str(&vb), fp) < 0)
      ohshite(_("failed to write %s database stanza about '%s' to '%s'"),
              which, pkgbin_name(pkg, pkgbin, pnaw_nonambig), filename);
    varbuf_reset(&vb);
  }

  pkg_array_destroy(&array);
  varbuf_destroy(&vb);
}

void
writedb(const char *filename, enum writedb_flags flags)
{
  struct atomic_file *file;
  enum atomic_file_flags atomic_flags = ATOMIC_FILE_BACKUP;

  if (flags & wdb_dump_available)
    atomic_flags = 0;

  file = atomic_file_new(filename, atomic_flags);
  atomic_file_open(file);

  writedb_stanzas(file->fp, filename, flags);

  if (flags & wdb_must_sync)
    atomic_file_sync(file);

  atomic_file_close(file);
  atomic_file_commit(file);
  atomic_file_free(file);

  if (flags & wdb_must_sync)
    dir_sync_path_parent(filename);
}
