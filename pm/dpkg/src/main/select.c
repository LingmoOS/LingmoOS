/*
 * dpkg - main program for package management
 * select.c - by-hand (rather than dselect-based) package selection
 *
 * Copyright © 1995,1996 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006, 2008-2015 Guillem Jover <guillem@debian.org>
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

#include <fnmatch.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/pkg-array.h>
#include <dpkg/pkg-show.h>
#include <dpkg/pkg-spec.h>
#include <dpkg/options.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>

#include "main.h"

static void getsel1package(struct pkginfo *pkg) {
  const char *pkgname;
  int l;

  if (pkg->want == PKG_WANT_UNKNOWN)
    return;
  pkgname = pkg_name(pkg, pnaw_nonambig);
  l = strlen(pkgname);
  l >>= 3;
  l = 6 - l;
  if (l < 1)
    l = 1;
  printf("%s%.*s%s\n", pkgname, l, "\t\t\t\t\t\t", pkg_want_name(pkg));
}

int
getselections(const char *const *argv)
{
  struct pkg_array array;
  struct pkginfo *pkg;
  int i;

  modstatdb_open(msdbrw_readonly);

  pkg_array_init_from_hash(&array);
  pkg_array_sort(&array, pkg_sorter_by_nonambig_name_arch);

  if (!*argv) {
    for (i = 0; i < array.n_pkgs; i++) {
      pkg = array.pkgs[i];
      if (pkg->status == PKG_STAT_NOTINSTALLED)
        continue;
      getsel1package(pkg);
    }
  } else {
    const char *thisarg;

    while ((thisarg= *argv++)) {
      struct pkg_spec pkgspec;
      int found;

      found= 0;
      pkg_spec_init(&pkgspec, PKG_SPEC_PATTERNS | PKG_SPEC_ARCH_WILDCARD);
      pkg_spec_parse(&pkgspec, thisarg);

      for (i = 0; i < array.n_pkgs; i++) {
        pkg = array.pkgs[i];
        if (!pkg_spec_match_pkg(&pkgspec, pkg, &pkg->installed))
          continue;
        getsel1package(pkg); found++;
      }
      if (!found)
        notice(_("no packages found matching %s"), thisarg);

      pkg_spec_destroy(&pkgspec);
    }
  }

  m_output(stdout, _("<standard output>"));
  m_output(stderr, _("<standard error>"));

  pkg_array_destroy(&array);

  modstatdb_shutdown();

  return 0;
}

int
setselections(const char *const *argv)
{
  enum modstatdb_rw msdbflags;
  const struct namevalue *nv;
  struct pkginfo *pkg;
  int c, lno;
  struct varbuf namevb = VARBUF_INIT;
  struct varbuf selvb = VARBUF_INIT;
  bool db_possibly_outdated = false;

  if (*argv)
    badusage(_("--%s takes no arguments"), cipaction->olong);

  msdbflags = msdbrw_available_readonly;
  if (!f_act)
    msdbflags |= msdbrw_readonly;
  else
    msdbflags |= msdbrw_write;

  modstatdb_open(msdbflags);
  pkg_infodb_upgrade();

  lno= 1;
  for (;;) {
    struct dpkg_error err;

    do {
      c = getchar();
      if (c == '\n')
        lno++;
    } while (c != EOF && c_isspace(c));
    if (c == EOF) break;
    if (c == '#') {
      do { c= getchar(); if (c == '\n') lno++; } while (c != EOF && c != '\n');
      continue;
    }

    varbuf_reset(&namevb);
    while (!c_isspace(c)) {
      varbuf_add_char(&namevb, c);
      c= getchar();
      if (c == EOF)
        ohshit(_("unexpected end of file in package name at line %d"), lno);
      if (c == '\n') ohshit(_("unexpected end of line in package name at line %d"),lno);
    }

    while (c != EOF && c_isspace(c)) {
      c= getchar();
      if (c == EOF)
        ohshit(_("unexpected end of file after package name at line %d"), lno);
      if (c == '\n') ohshit(_("unexpected end of line after package name at line %d"),lno);
    }

    varbuf_reset(&selvb);
    while (c != EOF && !c_isspace(c)) {
      varbuf_add_char(&selvb, c);
      c= getchar();
    }

    while (c != EOF && c != '\n') {
      c= getchar();
      if (!c_isspace(c))
        ohshit(_("unexpected data after package and selection at line %d"),lno);
    }
    pkg = pkg_spec_parse_pkg(varbuf_str(&namevb), &err);
    if (pkg == NULL)
      ohshit(_("illegal package name at line %d: %.250s"), lno, err.str);

    if (!pkg_is_informative(pkg, &pkg->installed) &&
        !pkg_is_informative(pkg, &pkg->available)) {
      db_possibly_outdated = true;
      warning(_("package not in status nor available database at line %d: %.250s"),
              lno, varbuf_str(&namevb));
      lno++;
      continue;
    }

    nv = namevalue_find_by_name(wantinfos, varbuf_str(&selvb));
    if (nv == NULL)
      ohshit(_("unknown wanted status at line %d: %.250s"),
             lno, varbuf_str(&selvb));

    pkg_set_want(pkg, nv->value);
    if (c == EOF) break;
    lno++;
  }
  if (ferror(stdin)) ohshite(_("read error on standard input"));
  modstatdb_shutdown();
  varbuf_destroy(&namevb);
  varbuf_destroy(&selvb);

  if (db_possibly_outdated)
    warning(_("found unknown packages; this might mean the available database\n"
              "is outdated, and needs to be updated through a frontend method;\n"
              "please see the FAQ <https://wiki.debian.org/Teams/Dpkg/FAQ#set-selections>"));

  return 0;
}

int
clearselections(const char *const *argv)
{
  enum modstatdb_rw msdbflags;
  struct pkg_hash_iter *iter;
  struct pkginfo *pkg;

  if (*argv)
    badusage(_("--%s takes no arguments"), cipaction->olong);

  if (!f_act)
    msdbflags = msdbrw_readonly;
  else
    msdbflags = msdbrw_write;

  modstatdb_open(msdbflags);
  pkg_infodb_upgrade();

  iter = pkg_hash_iter_new();
  while ((pkg = pkg_hash_iter_next_pkg(iter))) {
    if (!pkg->installed.essential &&
        !pkg->installed.is_protected &&
        pkg->want != PKG_WANT_UNKNOWN)
      pkg_set_want(pkg, PKG_WANT_DEINSTALL);
  }
  pkg_hash_iter_free(iter);

  modstatdb_shutdown();

  return 0;
}

