/*
 * dselect - Debian package maintenance user interface
 * pkgsublist.cc - status modification and recursive package list handling
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>

#include "dselect.h"
#include "bindings.h"

void packagelist::add(pkginfo *pkg) {
  debug(dbg_general, "packagelist[%p]::add(pkginfo %s)",
        this, pkg_name(pkg, pnaw_always));
  if (!recursive ||  // never add things to top level
      !pkg->clientdata ||  // don't add pure virtual packages
      pkg->clientdata->uprec)  // don't add ones already in the recursive list
    return;
  debug(dbg_general, "packagelist[%p]::add(pkginfo %s) adding",
        this, pkg_name(pkg, pnaw_always));
  perpackagestate *state= &datatable[nitems];
  state->pkg= pkg;
  state->direct= state->original= pkg->clientdata->selected;
  state->suggested= state->selected= pkg->clientdata->selected;
  state->spriority= sp_inherit; state->dpriority= dp_none;
  state->uprec= pkg->clientdata;
  state->relations.init();
  pkg->clientdata= state;
  table[nitems]= state;
  nitems++;
}

void
packagelist::add(pkginfo *pkg, pkgwant nw)
{
  debug(dbg_general, "packagelist[%p]::add(pkginfo %s, %s)",
        this, pkg_name(pkg, pnaw_always), wantstrings[nw]);
  add(pkg);  if (!pkg->clientdata) return;
  pkg->clientdata->direct= nw;
  selpriority np;
  np= would_like_to_install(nw,pkg) ? sp_selecting : sp_deselecting;
  if (pkg->clientdata->spriority > np) return;
  debug(dbg_general, "packagelist[%p]::add(pkginfo %s, %s) setting",
        this, pkg_name(pkg, pnaw_always), wantstrings[nw]);
  pkg->clientdata->suggested= pkg->clientdata->selected= nw;
  pkg->clientdata->spriority= np;
}

void packagelist::add(pkginfo *pkg, const char *extrainfo, showpriority showimp) {
  debug(dbg_general, "packagelist[%p]::add(pkginfo %s, ..., showpriority %d)",
        this, pkg_name(pkg, pnaw_always), showimp);
  add(pkg);  if (!pkg->clientdata) return;
  if (pkg->clientdata->dpriority < showimp) pkg->clientdata->dpriority= showimp;
  pkg->clientdata->relations += extrainfo;
}

bool
packagelist::alreadydone(doneent **done, void *check)
{
  doneent *search = *done;

  while (search && search->dep != check)
    search = search->next;
  if (search)
    return true;
  debug(dbg_general, "packagelist[%p]::alreadydone(%p, %p) new",
        this, done, check);
  search= new doneent;
  search->next= *done;
  search->dep= check;
  *done= search;
  return false;
}

void packagelist::addunavailable(deppossi *possi) {
  debug(dbg_general, "packagelist[%p]::addunavail(%p)", this, possi);

  if (!recursive) return;
  if (alreadydone(&unavdone,possi)) return;

  if (possi->up->up->clientdata == nullptr)
    internerr("deppossi from package %s has nullptr clientdata",
              pkg_name(possi->up->up, pnaw_always));
  if (possi->up->up->clientdata->uprec == nullptr)
    internerr("deppossi from package %s has nullptr clientdata's uprec",
              pkg_name(possi->up->up, pnaw_always));

  varbuf& vb= possi->up->up->clientdata->relations;
  vb += possi->ed->name;
  vb += _(" does not appear to be available\n");
}

bool
packagelist::add(dependency *depends, showpriority displayimportance)
{
  debug(dbg_general, "packagelist[%p]::add(dependency[%p])", this, depends);

  if (alreadydone(&depsdone, depends))
    return false;

  const char *comma= "";
  varbuf depinfo;
  depinfo += depends->up->set->name;
  depinfo += ' ';
  depinfo += gettext(relatestrings[depends->type]);
  depinfo += ' ';
  deppossi *possi;
  for (possi=depends->list;
       possi;
       possi=possi->next, comma=(possi && possi->next ? ", " : _(" or "))) {
    depinfo += comma;
    depinfo += possi->ed->name;
    if (possi->verrel != DPKG_RELATION_NONE) {
      switch (possi->verrel) {
      case DPKG_RELATION_LE:
        depinfo += " (<= ";
        break;
      case DPKG_RELATION_GE:
        depinfo += " (>= ";
        break;
      case DPKG_RELATION_LT:
        depinfo += " (<< ";
        break;
      case DPKG_RELATION_GT:
        depinfo += " (>> ";
        break;
      case DPKG_RELATION_EQ:
        depinfo += " (= ";
        break;
      default:
        internerr("unknown dpkg_relation %d", possi->verrel);
      }
      depinfo += versiondescribe(&possi->version, vdew_nonambig);
      depinfo += ")";
    }
  }
  depinfo += '\n';
  add(depends->up, depinfo.str(), displayimportance);
  for (possi=depends->list; possi; possi=possi->next) {
    add(&possi->ed->pkg, depinfo.str(), displayimportance);
    if (depends->type != dep_provides) {
      /* Providers are not relevant if we are looking at a provider
       * relationship already. */
      deppossi *provider;
      for (provider = possi->ed->depended.available;
           provider;
           provider = provider->rev_next) {
        if (provider->up->type != dep_provides) continue;
        add(provider->up->up, depinfo.str(), displayimportance);
        add(provider->up,displayimportance);
      }
    }
  }
  return true;
}

void repeatedlydisplay(packagelist *sub,
                       showpriority initial,
                       packagelist *unredisplay) {
  debug(dbg_general, "repeatedlydisplay(packagelist[%p])", sub);
  if (sub->resolvesuggest() != 0 && sub->deletelessimp_anyleft(initial)) {
    debug(dbg_general, "repeatedlydisplay(packagelist[%p]) once", sub);
    if (unredisplay) unredisplay->enddisplay();
    for (;;) {
      pkginfo **newl;
      keybindings *kb;

      /* Reset manual_install flag now that resolvesuggest() has seen it. */
      manual_install = false;
      newl= sub->display();
      if (!newl) break;
      debug(dbg_general, "repeatedlydisplay(packagelist[%p]) newl", sub);
      kb= sub->bindings; delete sub;
      sub= new packagelist(kb,newl);
      if (sub->resolvesuggest() <= 1) break;
      if (!sub->deletelessimp_anyleft(dp_must)) break;
      debug(dbg_general, "repeatedlydisplay(packagelist[%p]) again", sub);
    }
    if (unredisplay) unredisplay->startdisplay();
  }
  debug(dbg_general, "repeatedlydisplay(packagelist[%p]) done", sub);
  delete sub;
}

int packagelist::deletelessimp_anyleft(showpriority than) {
  debug(dbg_general, "packagelist[%p]::dli_al(%d): nitems=%d",
        this, than, nitems);
  int insat, runthr;
  for (runthr=0, insat=0;
       runthr < nitems;
       runthr++) {
    if (table[runthr]->dpriority < than) {
      table[runthr]->free(recursive);
    } else {
      if (insat != runthr) table[insat]= table[runthr];
      insat++;
    }
  }
  nitems= insat;
  debug(dbg_general, "packagelist[%p]::dli_al(%d) done; nitems=%d",
        this, than, nitems);
  return nitems;
}
