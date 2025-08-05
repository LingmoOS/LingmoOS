/*
 * dselect - Debian package maintenance user interface
 * pkgcmds.cc - package list keyboard commands
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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

#include <string.h>
#include <stdio.h>

#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>

#include "dselect.h"
#include "pkglist.h"

bool
packagelist::affectedmatches(struct pkginfo *pkg, struct pkginfo *comparewith) {
  switch (statsortorder) {
  case sso_avail:
    if (comparewith->clientdata->ssavail != pkg->clientdata->ssavail)
      return false;
    break;
  case sso_state:
    if (comparewith->clientdata->ssstate != pkg->clientdata->ssstate)
      return false;
    break;
  case sso_unsorted:
    break;
  default:
    internerr("unknown statsortorder %d", statsortorder);
  }
  if (comparewith->priority != PKG_PRIO_UNSET &&
      (comparewith->priority != pkg->priority ||
       (comparewith->priority == PKG_PRIO_OTHER &&
        strcasecmp(comparewith->otherpriority, pkg->otherpriority))))
    return false;
  if (comparewith->section &&
      strcasecmp(comparewith->section,
                 pkg->section ?
                 pkg->section : ""))
    return false;
  return true;
}

void packagelist::affectedrange(int *startp, int *endp) {
  if (table[cursorline]->pkg->set->name) {
    *startp= cursorline;
    *endp= cursorline+1;
    return;
  }
  int index = cursorline;
  while (index < nitems && !table[index]->pkg->set->name)
    index++;
  if (index >= nitems) {
    *startp= *endp= cursorline;
    return;
  }
  *startp= index;
  while (index < nitems && affectedmatches(table[index]->pkg,table[cursorline]->pkg))
    index++;
  *endp= index;
}

void packagelist::movecursorafter(int ncursor) {
  if (ncursor >= nitems) ncursor= nitems-1;
  topofscreen += ncursor-cursorline;
  if (topofscreen > nitems - list_height) topofscreen= nitems - list_height;
  if (topofscreen < 0) topofscreen= 0;
  setcursor(ncursor);
  refreshlist(); redrawthisstate();
}

pkgwant
packagelist::reallywant(pkgwant nwarg, struct perpackagestate *pkgstate)
{
  if (nwarg != PKG_WANT_SENTINEL)
    return nwarg;
  pkgstatus status = pkgstate->pkg->status;
  if (status == PKG_STAT_NOTINSTALLED)
    return PKG_WANT_PURGE;
  if (status == PKG_STAT_CONFIGFILES)
    return PKG_WANT_DEINSTALL;
  return PKG_WANT_INSTALL;
}

void
packagelist::setwant(pkgwant nwarg)
{
  int bot;

  if (modstatdb_get_status() == msdbrw_readonly) {
    beep();
    return;
  }

  if (recursive) {
    redrawitemsrange(cursorline,cursorline+1);
    table[cursorline]->selected= reallywant(nwarg,table[cursorline]);
    redraw1item(cursorline);
    bot= cursorline+1;
  } else {
    int top;
    pkgwant nw;

    packagelist *sub = new packagelist(bindings, nullptr);

    affectedrange(&top,&bot);
    for (int index = top; index < bot; index++) {
      if (!table[index]->pkg->set->name)
        continue;
      nw= reallywant(nwarg,table[index]);
      if (table[index]->selected == nw ||
          (table[index]->selected == PKG_WANT_PURGE &&
           nw == PKG_WANT_DEINSTALL))
        continue;
      sub->add(table[index]->pkg,nw);
    }

    repeatedlydisplay(sub,dp_may,this);
    for (int index = top; index < bot; index++)
      redraw1item(index);
  }
  movecursorafter(bot);
}

bool manual_install = false;

void packagelist::kd_select()   {
	manual_install = true;
	setwant(PKG_WANT_INSTALL);
	manual_install = false;
}
void packagelist::kd_hold()     { setwant(PKG_WANT_HOLD);      }
void packagelist::kd_deselect() { setwant(PKG_WANT_DEINSTALL); }
void packagelist::kd_unhold()   { setwant(PKG_WANT_SENTINEL);  }
void packagelist::kd_purge()    { setwant(PKG_WANT_PURGE);     }

int
would_like_to_install(pkgwant wantvalue, pkginfo *pkg)
{
  /* Returns: 1 for yes, 0 for no, -1 for if they want to preserve an error condition. */
  debug(dbg_general, "would_like_to_install(%d, %s) status %d",
        wantvalue, pkg_name(pkg, pnaw_always), pkg->status);

  if (wantvalue == PKG_WANT_INSTALL)
    return 1;
  if (wantvalue != PKG_WANT_HOLD)
    return 0;
  if (pkg->status == PKG_STAT_INSTALLED)
    return 1;
  if (pkg->status == PKG_STAT_NOTINSTALLED ||
      pkg->status == PKG_STAT_CONFIGFILES)
    return 0;
  return -1;
}

const char *packagelist::itemname(int index) {
  return table[index]->pkg->set->name;
}

void packagelist::kd_swapstatorder() {
  if (sortorder == so_unsorted) return;
  switch (statsortorder) {
  case sso_avail:     statsortorder= sso_state;     break;
  case sso_state:     statsortorder= sso_unsorted;  break;
  case sso_unsorted:  statsortorder= sso_avail;     break;
  default:
    internerr("unknown statsort %d", statsortorder);
  }
  resortredisplay();
}

void packagelist::kd_swaporder() {
  switch (sortorder) {
  case so_priority:  sortorder= so_section;   break;
  case so_section:   sortorder= so_alpha;     break;
  case so_alpha:     sortorder= so_priority;  break;
  case so_unsorted:                           return;
  default:
    internerr("unknown sort %d", sortorder);
  }
  resortredisplay();
}

void packagelist::resortredisplay() {
  const char *oldname = table[cursorline]->pkg->set->name;
  sortmakeheads();
  int newcursor;
  newcursor= 0;
  if (oldname) {
    int index;
    for (index=0; index<nitems; index++) {
      if (table[index]->pkg->set->name &&
          strcasecmp(oldname, table[index]->pkg->set->name) == 0) {
        newcursor= index;
        break;
      }
    }
  }
  topofscreen= newcursor-1;
  if (topofscreen > nitems - list_height) topofscreen= nitems-list_height;
  if (topofscreen < 0) topofscreen= 0;
  setwidths();
  redrawtitle();
  redrawcolheads();
  ldrawnstart= ldrawnend= -1;
  cursorline= -1;
  setcursor(newcursor);
  refreshlist();
}

void
packagelist::kd_archdisplay()
{
  switch (archdisplayopt) {
  case ado_both:
    archdisplayopt = ado_none;
    break;
  case ado_none:
    archdisplayopt = ado_available;
    break;
  case ado_available:
    archdisplayopt = ado_both;
    break;
  default:
    internerr("unknown archdisplayopt %d", archdisplayopt);
  }
  setwidths();
  leftofscreen = 0;
  ldrawnstart = ldrawnend = -1;
  redrawtitle();
  redrawcolheads();
  redrawitemsrange(topofscreen, min(topofscreen + list_height, nitems));
  refreshlist();
}

void packagelist::kd_versiondisplay() {
  switch (versiondisplayopt) {
  case vdo_both:       versiondisplayopt= vdo_none;       break;
  case vdo_none:       versiondisplayopt= vdo_available;  break;
  case vdo_available:  versiondisplayopt= vdo_both;       break;
  default:
    internerr("unknown versiondisplayopt %d", versiondisplayopt);
  }
  setwidths();
  leftofscreen= 0;
  ldrawnstart= ldrawnend= -1;
  redrawtitle();
  redrawcolheads();
  redrawitemsrange(topofscreen, min(topofscreen + list_height, nitems));
  refreshlist();
}

void packagelist::kd_verbose() {
  verbose= !verbose;
  setwidths();
  leftofscreen= 0;
  ldrawnstart= ldrawnend= -1;
  redrawtitle();
  redrawcolheads();
  redrawitemsrange(topofscreen, min(topofscreen + list_height, nitems));
  refreshlist();
}

void packagelist::kd_quit_noop() { }

void packagelist::kd_revert_abort() {
  int index;
  for (index=0; index<nitems; index++) {
    if (table[index]->pkg->set->name)
      table[index]->selected= table[index]->original;
    ldrawnstart= ldrawnend= -1;
  }
  refreshlist(); redrawthisstate();
}

void packagelist::kd_revertdirect() {
  int index;
  for (index=0; index<nitems; index++) {
    if (table[index]->pkg->set->name)
      table[index]->selected= table[index]->direct;
    ldrawnstart= ldrawnend= -1;
  }
  refreshlist(); redrawthisstate();
}

void packagelist::kd_revertsuggest() {
  int index;
  for (index=0; index<nitems; index++) {
    if (table[index]->pkg->set->name)
      table[index]->selected= table[index]->suggested;
    ldrawnstart= ldrawnend= -1;
  }
  refreshlist(); redrawthisstate();
}

void
packagelist::kd_revertinstalled()
{
  int i;

  for (i = 0; i < nitems; i++) {
    if (table[i]->pkg->set->name)
      table[i]->selected = reallywant(PKG_WANT_SENTINEL, table[i]);
    ldrawnstart = ldrawnend = -1;
  }

  refreshlist();
  redrawthisstate();
}

/* FIXME: configurable purge/deselect */

void packagelist::kd_toggleinfo() {
  showinfo= (showinfo+2) % 3;
  setheights();
  if (cursorline >= topofscreen+list_height) topofscreen += list_height;
  if (topofscreen > nitems - list_height) topofscreen= nitems-list_height;
  if (topofscreen < 0) topofscreen= 0;
  infotopofscreen= 0;
  redraw1item(cursorline);
  refreshlist();
  redrawthisstate();
  redrawinfo();
}

void packagelist::kd_info() {
  if (!showinfo) {
    showinfo= 2; kd_toggleinfo();
  } else {
    currentinfo++;
    infotopofscreen=0;
    redrawinfo();
  }
}
