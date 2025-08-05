/*
 * dselect - Debian package maintenance user interface
 * pkgtop.cc - handles (re)draw of package list windows colheads, list, thisstate
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2007-2014 Guillem Jover <guillem@debian.org>
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

#include <string.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>

#include "dselect.h"
#include "pkglist.h"

static const char *
pkgprioritystring(const struct pkginfo *pkg)
{
  if (pkg->priority == PKG_PRIO_UNSET) {
    return nullptr;
  } else if (pkg->priority == PKG_PRIO_OTHER) {
    return pkg->otherpriority;
  } else {
    if (pkg->priority > PKG_PRIO_UNKNOWN)
      internerr("package %s has out-of-range priority %d",
                pkg_name_const(pkg, pnaw_always), pkg->priority);
    return gettext(prioritystrings[pkg->priority]);
  }
}

int
packagelist::describemany(varbuf &vb,
                          const char *prioritystring, const char *section,
                          const struct perpackagestate *pps)
{
  const char *ssostring, *ssoabbrev;
  int statindent;

  statindent= 0;
  ssostring = nullptr;
  ssoabbrev= _("All");
  switch (statsortorder) {
  case sso_avail:
    if (pps->ssavail == ssa_none)
      break;
    ssostring= ssastrings[pps->ssavail];
    ssoabbrev= ssaabbrevs[pps->ssavail];
    statindent++;
    break;
  case sso_state:
    if (pps->ssstate == sss_none)
      break;
    ssostring= sssstrings[pps->ssstate];
    ssoabbrev= sssabbrevs[pps->ssstate];
    statindent++;
    break;
  case sso_unsorted:
    break;
  default:
    internerr("unknown statsortrder %d", statsortorder);
  }

  vb.reset();
  if (!prioritystring) {
    if (!section) {
      vb += ssostring ? gettext(ssostring) : _("All packages");
      return statindent;
    } else {
      if (!*section) {
        vb.add_fmt(_("%s packages without a section"), gettext(ssoabbrev));
      } else {
        vb.add_fmt(_("%s packages in section %s"), gettext(ssoabbrev), section);
      }
      return statindent+1;
    }
  } else {
    if (!section) {
      vb.add_fmt(_("%s %s packages"), gettext(ssoabbrev), prioritystring);
      return statindent+1;
    } else {
      if (!*section) {
        vb.add_fmt(_("%s %s packages without a section"), gettext(ssoabbrev), prioritystring);
      } else {
        vb.add_fmt(_("%s %s packages in section %s"), gettext(ssoabbrev), prioritystring, section);
      }
      return statindent+2;
    }
  }
}

void packagelist::redrawthisstate() {
  if (!thisstate_height) return;
  mywerase(thisstatepad);

  const char *section= table[cursorline]->pkg->section;
  const char *priority= pkgprioritystring(table[cursorline]->pkg);
  varbuf vb;

  if (table[cursorline]->pkg->set->name) {
    vb.add_fmt(_("%-*s %s%s%s;  %s (was: %s).  %s"),
               col_package.width,
               table[cursorline]->pkg->set->name,
               gettext(statusstrings[table[cursorline]->pkg->status]),
               ((eflagstrings[table[cursorline]->pkg->eflag][0] == ' ') &&
                (eflagstrings[table[cursorline]->pkg->eflag][1] == '\0'))  ? "" : " - ",
               gettext(eflagstrings[table[cursorline]->pkg->eflag]),
               gettext(wantstrings[table[cursorline]->selected]),
               gettext(wantstrings[table[cursorline]->original]),
               priority);
  } else {
    describemany(vb, priority, section, table[cursorline]->pkg->clientdata);
  }
  mvwaddnstr(thisstatepad, 0, 0, vb.str(), total_width);
  pnoutrefresh(thisstatepad, 0,leftofscreen, thisstate_row,0,
               thisstate_row, min(total_width - 1, xmax - 1));
}

void packagelist::redraw1itemsel(int index, int selected) {
  int i;
  const char *p;
  const struct pkginfo *pkg= table[index]->pkg;
  int screenline = index - topofscreen;

  wattrset(listpad, part_attr[selected ? listsel : list]);

  if (pkg->set->name) {
    if (verbose) {
      draw_column_item(col_status_hold, screenline,
                       gettext(eflagstrings[pkg->eflag]));

      draw_column_sep(col_status_status, screenline);
      draw_column_item(col_status_status, screenline,
                       gettext(statusstrings[pkg->status]));

      draw_column_sep(col_status_old_want, screenline);
      draw_column_item(col_status_old_want, screenline,
              /* FIXME: keep this? */
              /*table[index]->original == table[index]->selected ? "(same)"
              : */gettext(wantstrings[table[index]->original]));

      draw_column_sep(col_status_new_want, screenline);
      wattrset(listpad, part_attr[selected ? selstatesel : selstate]);
      draw_column_item(col_status_new_want, screenline,
                       gettext(wantstrings[table[index]->selected]));

      wattrset(listpad, part_attr[selected ? listsel : list]);

      draw_column_sep(col_priority, screenline);
      draw_column_item(col_priority, screenline,
                       pkg->priority == PKG_PRIO_OTHER ?
                       pkg->otherpriority :
                       gettext(prioritystrings[pkg->priority]));
    } else {
      mvwaddch(listpad, screenline, 0, eflagchars[pkg->eflag]);
      waddch(listpad, statuschars[pkg->status]);
      waddch(listpad,
             /* FIXME: keep this feature? */
             /*table[index]->original == table[index]->selected ? ' '
             : */wantchars[table[index]->original]);

      wattrset(listpad, part_attr[selected ? selstatesel : selstate]);
      waddch(listpad, wantchars[table[index]->selected]);
      wattrset(listpad, part_attr[selected ? listsel : list]);

      wmove(listpad, screenline, col_priority.x - 1);
      waddch(listpad, ' ');
      if (pkg->priority == PKG_PRIO_OTHER) {
        for (i = col_priority.width, p = pkg->otherpriority;
             i > 0 && *p;
             i--, p++)
          waddch(listpad, c_tolower(*p));
        while (i-- > 0) waddch(listpad,' ');
      } else {
        wprintw(listpad, "%-*.*s", col_priority.width, col_priority.width,
                gettext(priorityabbrevs[pkg->priority]));
      }
    }

    draw_column_sep(col_section, screenline);
    draw_column_item(col_section, screenline,
                     pkg->section ? pkg->section : "?");

    draw_column_sep(col_package, screenline);
    draw_column_item(col_package, screenline,
                     pkg->set->name);

    waddch(listpad, ' ');

    if (col_archinstalled.width) {
      draw_column_sep(col_archinstalled, screenline);
      draw_column_item(col_archinstalled, screenline, pkg->installed.arch->name);

      waddch(listpad, ' ');
    }
    if (col_archavailable.width) {
      draw_column_sep(col_archavailable, screenline);
      draw_column_item(col_archavailable, screenline, pkg->available.arch->name);

      waddch(listpad, ' ');
    }

    if (col_versioninstalled.width) {
      draw_column_item(col_versioninstalled, screenline,
                       versiondescribe(&pkg->installed.version, vdew_nonambig));
      waddch(listpad, ' ');
    }
    if (col_versionavailable.width) {
      if (dpkg_version_is_informative(&pkg->available.version) &&
          dpkg_version_compare(&pkg->available.version,
                               &pkg->installed.version) > 0)
        wattrset(listpad, part_attr[selected ? selstatesel : selstate]);
      draw_column_item(col_versionavailable, screenline,
                       versiondescribe(&pkg->available.version, vdew_nonambig));
      wattrset(listpad, part_attr[selected ? listsel : list]);
      waddch(listpad,' ');
    }

    i = col_description.width;
    p = pkg->available.description ? pkg->available.description :
        pkg->installed.description ? pkg->installed.description : "";
    while (i>0 && *p && *p != '\n') { waddnstr(listpad,p,1); i--; p++; }
  } else {
    int j, indent;
    const char *section= pkg->section;
    const char *priority= pkgprioritystring(pkg);

    varbuf vb;

    indent = describemany(vb, priority, section, pkg->clientdata);

    mvwaddstr(listpad, screenline, 0, "    ");
    i= total_width-7;
    j= (indent<<1) + 1;
    while (j-- >0) { waddch(listpad,ACS_HLINE); i--; }
    waddch(listpad,' ');

    wattrset(listpad, part_attr[selected ? selstatesel : selstate]);
    p = vb.str();
    while (i>0 && *p) { waddnstr(listpad, p,1); p++; i--; }
    wattrset(listpad, part_attr[selected ? listsel : list]);

    waddch(listpad,' ');
    j= (indent<<1) + 1;
    while (j-- >0) { waddch(listpad,ACS_HLINE); i--; }
  }

  while (i>0) { waddch(listpad,' '); i--; }
}

void packagelist::redrawcolheads() {
  if (colheads_height) {
    wattrset(colheadspad, part_attr[colheads]);
    mywerase(colheadspad);
    if (verbose) {
      wmove(colheadspad,0,0);
      for (int i = 0; i < col_status_old_want.width; i++)
        waddch(colheadspad, '.');
      draw_column_head(col_status_hold);
      draw_column_head(col_status_status);
      draw_column_head(col_status_old_want);
      draw_column_head(col_status_new_want);
    } else {
      draw_column_head(col_status);
    }

    draw_column_head(col_section);
    draw_column_head(col_priority);
    draw_column_head(col_package);

    if (col_archinstalled.width)
      draw_column_head(col_archinstalled);
    if (col_archavailable.width)
      draw_column_head(col_archavailable);

    if (col_versioninstalled.width)
      draw_column_head(col_versioninstalled);
    if (col_versionavailable.width)
      draw_column_head(col_versionavailable);

    draw_column_head(col_description);
  }
  refreshcolheads();
}
