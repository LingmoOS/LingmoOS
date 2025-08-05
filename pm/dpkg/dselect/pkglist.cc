/*
 * dselect - Debian package maintenance user interface
 * pkglist.cc - package list administration
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2001 Wichert Akkerman <wakkerma@debian.org>
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

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/string.h>

#include "dselect.h"
#include "bindings.h"

int packagelist::compareentries(const struct perpackagestate *a,
                                const struct perpackagestate *b) {
  switch (statsortorder) {
  case sso_avail:
    if (a->ssavail != b->ssavail) return a->ssavail - b->ssavail;
    break;
  case sso_state:
    if (a->ssstate != b->ssstate) return a->ssstate - b->ssstate;
    break;
  case sso_unsorted:
    break;
  default:
    internerr("unknown statsortorder %d", statsortorder);
  }

  const char *asection= a->pkg->section;
  if (!asection && a->pkg->set->name)
    asection = "";
  const char *bsection= b->pkg->section;
  if (!bsection && b->pkg->set->name)
    bsection = "";
  int c_section=
    !asection || !bsection ?
      (!bsection) - (!asection) :
    !*asection || !*bsection ?
      (!*asection) - (!*bsection) :
    strcasecmp(asection,bsection);
  int c_priority=
    a->pkg->priority - b->pkg->priority;
  if (!c_priority && a->pkg->priority == PKG_PRIO_OTHER)
    c_priority= strcasecmp(a->pkg->otherpriority, b->pkg->otherpriority);
  int c_name=
    a->pkg->set->name && b->pkg->set->name ?
      strcasecmp(a->pkg->set->name, b->pkg->set->name) :
    (!b->pkg->set->name) - (!a->pkg->set->name);

  switch (sortorder) {
  case so_section:
    return c_section ? c_section : c_priority ? c_priority : c_name;
  case so_priority:
    return c_priority ? c_priority : c_section ? c_section : c_name;
  case so_alpha:
    return c_name;
  case so_unsorted:
  default:
    internerr("unsorted or unknown sort %d", sortorder);
  }
  /* never reached, make gcc happy */
  return 1;
}

void packagelist::discardheadings() {
  int a,b;
  for (a=0, b=0; a<nitems; a++) {
    if (table[a]->pkg->set->name) {
      table[b++]= table[a];
    }
  }
  nitems= b;

  struct perpackagestate *head, *next;
  head= headings;
  while (head) {
    next= head->uprec;
    delete head->pkg->set;
    delete head;
    head= next;
  }
  headings = nullptr;
}

void packagelist::addheading(enum ssavailval ssavail,
                             enum ssstateval ssstate,
                             pkgpriority priority,
                             const char *otherpriority,
                             const char *section) {
  if (nitems > nallocated)
    internerr("inconsistent state: ntimes=%d > nallocated=%d",
              nitems, nallocated);

  if (nitems == nallocated) {
    nallocated += nallocated+50;
    struct perpackagestate **newtable= new struct perpackagestate*[nallocated];
    memcpy(newtable, table, nallocated * sizeof(struct perpackagestate *));
    delete[] table;
    table= newtable;
  }

  debug(dbg_general, "packagelist[%p]::addheading(%d,%d,%d,%s,%s)",
        this, ssavail, ssstate, priority,
        otherpriority ? otherpriority : "<null>",
        section ? section : "<null>");

  struct pkgset *newset = new pkgset;
  newset->name = nullptr;
  struct pkginfo *newhead = &newset->pkg;
  newhead->set = newset;
  newhead->priority= priority;
  newhead->otherpriority= otherpriority;
  newhead->section= section;

  struct perpackagestate *newstate= new perpackagestate;
  newstate->pkg= newhead;
  newstate->uprec= headings;
  headings= newstate;
  newstate->ssavail= ssavail;
  newstate->ssstate= ssstate;
  newhead->clientdata= newstate;

  table[nitems++]= newstate;
}

static packagelist *sortpackagelist;

int qsort_compareentries(const void *a, const void *b) {
  const struct perpackagestate *pa = *static_cast<const struct perpackagestate * const *>(a);
  const struct perpackagestate *pb = *static_cast<const struct perpackagestate * const *>(b);

  return sortpackagelist->compareentries(pa, pb);
}

void packagelist::sortinplace() {
  sortpackagelist= this;

  debug(dbg_general, "packagelist[%p]::sortinplace()", this);
  qsort(table, nitems, sizeof(struct pkgbin *), qsort_compareentries);
}

void packagelist::ensurestatsortinfo() {
  const struct dpkg_version *veri;
  const struct dpkg_version *vera;
  struct pkginfo *pkg;
  int index;

  debug(dbg_general,
        "packagelist[%p]::ensurestatsortinfos() sortorder=%d nitems=%d",
        this, statsortorder, nitems);

  switch (statsortorder) {
  case sso_unsorted:
    debug(dbg_general, "packagelist[%p]::ensurestatsortinfos() unsorted", this);
    return;
  case sso_avail:
    debug(dbg_general, "packagelist[%p]::ensurestatsortinfos() calcssadone=%d",
          this, calcssadone);
    if (calcssadone) return;
    for (index=0; index < nitems; index++) {
      debug(dbg_general, "packagelist[%p]::ensurestatsortinfos() i=%d pkg=%s",
            this, index, pkg_name(table[index]->pkg, pnaw_always));
      pkg= table[index]->pkg;
      switch (pkg->status) {
      case PKG_STAT_UNPACKED:
      case PKG_STAT_HALFCONFIGURED:
      case PKG_STAT_HALFINSTALLED:
      case PKG_STAT_TRIGGERSAWAITED:
      case PKG_STAT_TRIGGERSPENDING:
        table[index]->ssavail= ssa_broken;
        break;
      case PKG_STAT_NOTINSTALLED:
      case PKG_STAT_CONFIGFILES:
        if (!dpkg_version_is_informative(&pkg->available.version)) {
          table[index]->ssavail= ssa_notinst_gone;
// FIXME: Disable for now as a workaround, until dselect knows how to properly
//        store seen packages.
#if 0
        } else if (table[index]->original == PKG_WANT_UNKNOWN) {
          table[index]->ssavail= ssa_notinst_unseen;
#endif
        } else {
          table[index]->ssavail= ssa_notinst_seen;
        }
        break;
      case PKG_STAT_INSTALLED:
        veri= &table[index]->pkg->installed.version;
        vera= &table[index]->pkg->available.version;
        if (!dpkg_version_is_informative(vera)) {
          table[index]->ssavail= ssa_installed_gone;
        } else if (dpkg_version_compare(vera, veri) > 0) {
          table[index]->ssavail= ssa_installed_newer;
        } else {
          table[index]->ssavail= ssa_installed_sameold;
        }
        break;
      default:
        internerr("unknown status %d on sso_avail", pkg->status);
      }
      debug(dbg_general,
            "packagelist[%p]::ensurestatsortinfos() i=%d ssavail=%d",
            this, index, table[index]->ssavail);
    }
    calcssadone = true;
    break;
  case sso_state:
    debug(dbg_general, "packagelist[%p]::ensurestatsortinfos() calcsssdone=%d",
          this, calcsssdone);
    if (calcsssdone) return;
    for (index=0; index < nitems; index++) {
      debug(dbg_general, "packagelist[%p]::ensurestatsortinfos() i=%d pkg=%s",
            this, index, pkg_name(table[index]->pkg, pnaw_always));
      switch (table[index]->pkg->status) {
      case PKG_STAT_UNPACKED:
      case PKG_STAT_HALFCONFIGURED:
      case PKG_STAT_HALFINSTALLED:
      case PKG_STAT_TRIGGERSAWAITED:
      case PKG_STAT_TRIGGERSPENDING:
        table[index]->ssstate= sss_broken;
        break;
      case PKG_STAT_NOTINSTALLED:
        table[index]->ssstate= sss_notinstalled;
        break;
      case PKG_STAT_CONFIGFILES:
        table[index]->ssstate= sss_configfiles;
        break;
      case PKG_STAT_INSTALLED:
        table[index]->ssstate= sss_installed;
        break;
      default:
        internerr("unknown status %d on sso_state", table[index]->pkg->status);
      }
      debug(dbg_general,
            "packagelist[%p]::ensurestatsortinfos() i=%d ssstate=%d",
            this, index, table[index]->ssstate);
    }
    calcsssdone = true;
    break;
  default:
    internerr("unknown statsortorder %d", statsortorder);
  }
}

void packagelist::sortmakeheads() {
  discardheadings();
  ensurestatsortinfo();
  sortinplace();

  if (nitems == 0)
    internerr("cannot sort 0 items");

  debug(dbg_general,
        "packagelist[%p]::sortmakeheads() sortorder=%d statsortorder=%d",
        this, sortorder, statsortorder);

  int nrealitems= nitems;
  addheading(ssa_none, sss_none, PKG_PRIO_UNSET, nullptr, nullptr);

  if (sortorder == so_unsorted)
    internerr("cannot sort unsorted order");

  if (sortorder == so_alpha && statsortorder == sso_unsorted) { sortinplace(); return; }

  // Important: do not save pointers into table in this function, because
  // addheading may need to reallocate table to make it larger !

  struct pkginfo *lastpkg;
  struct pkginfo *thispkg;
  lastpkg = nullptr;
  int a;
  for (a=0; a<nrealitems; a++) {
    thispkg= table[a]->pkg;
    if (thispkg->set->name == nullptr)
      internerr("package set has no name at table index %d", a);
    int ssdiff= 0;
    ssavailval ssavail= ssa_none;
    ssstateval ssstate= sss_none;
    switch (statsortorder) {
    case sso_avail:
      ssavail= thispkg->clientdata->ssavail;
      ssdiff= (!lastpkg || ssavail != lastpkg->clientdata->ssavail);
      break;
    case sso_state:
      ssstate= thispkg->clientdata->ssstate;
      ssdiff= (!lastpkg || ssstate != lastpkg->clientdata->ssstate);
      break;
    case sso_unsorted:
      break;
    default:
      internerr("unknown statsortorder %d", statsortorder);
    }

    int prioritydiff= (!lastpkg ||
                       thispkg->priority != lastpkg->priority ||
                       (thispkg->priority == PKG_PRIO_OTHER &&
                        strcasecmp(thispkg->otherpriority,lastpkg->otherpriority)));
    int sectiondiff= (!lastpkg ||
                      strcasecmp(thispkg->section ? thispkg->section : "",
                                 lastpkg->section ? lastpkg->section : ""));

    debug(dbg_general,
          "packagelist[%p]::sortmakeheads() pkg=%s  state=%d avail=%d %s  "
          "priority=%d otherpriority=%s %s  section=%s %s",
          this, pkg_name(thispkg, pnaw_always),
          thispkg->clientdata->ssavail, thispkg->clientdata->ssstate,
          ssdiff ? "*diff" : "same",
          thispkg->priority,
          thispkg->priority != PKG_PRIO_OTHER ? "<none>" :
          thispkg->otherpriority ? thispkg->otherpriority : "<null>",
          prioritydiff ? "*diff*" : "same",
          thispkg->section ? thispkg->section : "<null>",
          sectiondiff ? "*diff*" : "same");

    if (ssdiff)
      addheading(ssavail,ssstate,
                 PKG_PRIO_UNSET, nullptr, nullptr);

    if (sortorder == so_section && sectiondiff)
      addheading(ssavail,ssstate,
                 PKG_PRIO_UNSET, nullptr,
                 thispkg->section ? thispkg->section : "");

    if (sortorder == so_priority && prioritydiff)
      addheading(ssavail,ssstate,
                 thispkg->priority, thispkg->otherpriority, nullptr);

    if (sortorder != so_alpha && (prioritydiff || sectiondiff))
      addheading(ssavail,ssstate,
                 thispkg->priority,thispkg->otherpriority,
                 thispkg->section ? thispkg->section : "");

    lastpkg= thispkg;
  }

  if (listpad) {
    werase(listpad);
  }

  sortinplace();
}

void packagelist::initialsetup() {
  debug(dbg_general, "packagelist[%p]::initialsetup()", this);

  int allpackages = pkg_hash_count_pkg();
  datatable= new struct perpackagestate[allpackages];

  nallocated= allpackages+150; // will realloc if necessary, so 150 not critical
  table= new struct perpackagestate*[nallocated];

  depsdone = nullptr;
  unavdone = nullptr;
  currentinfo = nullptr;
  headings = nullptr;
  verbose = false;
  calcssadone = calcsssdone = false;
  searchdescr = false;
}

void packagelist::finalsetup() {
  setcursor(0);

  debug(dbg_general, "packagelist[%p]::finalsetup done; recursive=%d nitems=%d",
        this, recursive, nitems);
}

packagelist::packagelist(keybindings *kb) : baselist(kb) {
  // nonrecursive
  initialsetup();
  struct pkg_hash_iter *iter;
  struct pkginfo *pkg;

  nitems = 0;

  iter = pkg_hash_iter_new();
  while ((pkg = pkg_hash_iter_next_pkg(iter))) {
    struct perpackagestate *state= &datatable[nitems];
    state->pkg= pkg;
    if (pkg->status == PKG_STAT_NOTINSTALLED &&
        !pkg->archives &&
        pkg->want != PKG_WANT_INSTALL) {
      pkg->clientdata = nullptr;
      continue;
    }
    // treat all unknown packages as already seen
    state->direct = state->original = (pkg->want == PKG_WANT_UNKNOWN ? PKG_WANT_PURGE : pkg->want);
    if (modstatdb_get_status() == msdbrw_write &&
        state->original == PKG_WANT_UNKNOWN) {
      state->suggested=
        pkg->status == PKG_STAT_INSTALLED ||
          pkg->priority <= PKG_PRIO_STANDARD /* FIXME: configurable */
            ? PKG_WANT_INSTALL : PKG_WANT_PURGE;
      state->spriority= sp_inherit;
    } else {
      state->suggested= state->original;
      state->spriority= sp_fixed;
    }
    state->dpriority= dp_must;
    state->selected= state->suggested;
    state->uprec = nullptr;
    state->relations.init();
    pkg->clientdata= state;
    table[nitems]= state;
    nitems++;
  }
  pkg_hash_iter_free(iter);

  if (!nitems)
    ohshit(_("there are no packages"));
  recursive = false;
  sortorder= so_priority;
  statsortorder= sso_avail;
  archdisplayopt = ado_both;
  versiondisplayopt= vdo_both;
  sortmakeheads();
  finalsetup();
}

packagelist::packagelist(keybindings *kb, pkginfo **pkgltab) : baselist(kb) {
  // takes over responsibility for pkgltab (recursive)
  initialsetup();

  recursive = true;
  nitems= 0;
  if (pkgltab) {
    add(pkgltab);
    delete[] pkgltab;
  }

  sortorder= so_unsorted;
  statsortorder= sso_unsorted;
  archdisplayopt = ado_none;
  versiondisplayopt= vdo_none;
  finalsetup();
}

void
perpackagestate::free(bool recursive)
{
  if (pkg->set->name) {
    if (modstatdb_get_status() == msdbrw_write) {
      if (uprec) {
        if (!recursive)
          internerr("unexpected non-recursive free requested");
        uprec->selected= selected;
        pkg->clientdata= uprec;
      } else {
        if (recursive)
          internerr("unexpected recursive free requested");
        if (pkg->want != selected &&
            !(pkg->want == PKG_WANT_UNKNOWN && selected == PKG_WANT_PURGE)) {
          pkg->want= selected;
        }
        pkg->clientdata = nullptr;
      }
    }
    relations.destroy();
  }
}

packagelist::~packagelist() {
  debug(dbg_general, "packagelist[%p]::~packagelist()", this);

  if (searchstring.len())
    regfree(&searchfsm);

  discardheadings();

  int index;
  for (index=0; index<nitems; index++) table[index]->free(recursive);
  delete[] table;
  delete[] datatable;
  debug(dbg_general, "packagelist[%p]::~packagelist() tables freed", this);

  doneent *search, *next;
  for (search=depsdone; search; search=next) {
    next= search->next;
    delete search;
  }

  debug(dbg_general, "packagelist[%p]::~packagelist() done", this);
}

bool
packagelist::checksearch(varbuf &rx)
{
  int rc, opt = REG_NOSUB;
  int pos;

  if (rx.len() == 0)
    return false;

  searchdescr = false;
  if (searchstring.len()) {
    regfree(&searchfsm);
    searchstring.reset();
  }

  /* look for search options */
  for (pos = rx.len() - 1; pos >= 0; pos--)
    if ((rx[pos] == '/') && ((pos == 0) || (rx[pos - 1] != '\\')))
      break;

  if (pos >= 0) {
    rx[pos++] = '\0';
    if (strcspn(rx.str() + pos, "di") != 0) {
      displayerror(_("invalid search option given"));
      return false;
    }

   while (rx[pos]) {
     if (rx[pos] == 'i')
       opt|=REG_ICASE;
     else if (rx[pos] == 'd')
       searchdescr = true;
     pos++;
   }
  }

  rc = regcomp(&searchfsm, rx.str(), opt);
  if (rc != 0) {
    displayerror(_("error in regular expression"));
    return false;
  }

  return true;
}

bool
packagelist::matchsearch(int index)
{
  const char *name;

  name = itemname(index);
  if (!name)
    return false;	/* Skip things without a name (separators) */

  if (regexec(&searchfsm, name, 0, nullptr, 0) == 0)
    return true;

  if (searchdescr) {
    const char *descr = table[index]->pkg->available.description;
    if (str_is_unset(descr))
      return false;

    if (regexec(&searchfsm, descr, 0, nullptr, 0) == 0)
      return true;
  }

  return false;
}

pkginfo **packagelist::display() {
  // returns list of packages as null-terminated array, which becomes owned
  // by the caller, if a recursive check is desired.
  // returns 0 if no recursive check is desired.
  int response, index;
  const keybindings::interpretation *interp;

  debug(dbg_general, "packagelist[%p]::display()", this);

  startdisplay();

  if (!expertmode)
  displayhelp(helpmenulist(),'i');

  debug(dbg_general, "packagelist[%p]::display() entering loop", this);
  for (;;) {
    if (whatinfo_height) wcursyncup(whatinfowin);
    if (doupdate() == ERR)
      ohshite(_("doupdate failed"));
    do {
      response = getch();
      if (response == KEY_RESIZE) {
        resize_window();
        continue;
      }
    } while (response == ERR && errno == EINTR);
    if (response == ERR)
      ohshite(_("getch failed"));
    interp= (*bindings)(response);
    debug(dbg_general, "packagelist[%p]::display() response=%d interp=%s",
          this, response, interp ? interp->action : "[none]");
    if (!interp)
      continue;
    (this->*(interp->pfn))();
    if (interp->qa != qa_noquit) break;
  }
  enddisplay();

  if (interp->qa == qa_quitnochecksave ||
      modstatdb_get_status() == msdbrw_readonly) {
    debug(dbg_general, "packagelist[%p]::display() done - quitNOcheck", this);
    return nullptr;
  }

  if (recursive) {
    pkginfo **retl = new pkginfo*[nitems + 1];
    for (index=0; index<nitems; index++) retl[index]= table[index]->pkg;
    retl[nitems] = nullptr;
    debug(dbg_general, "packagelist[%p]::display() done, retl=%p", this, retl);
    return retl;
  } else {
    packagelist *sub = new packagelist(bindings, nullptr);
    for (index=0; index < nitems; index++)
      if (table[index]->pkg->set->name)
        sub->add(table[index]->pkg);
    repeatedlydisplay(sub,dp_must);
    debug(dbg_general,
          "packagelist[%p]::display() done, not recursive no retl", this);
    return nullptr;
  }
}
