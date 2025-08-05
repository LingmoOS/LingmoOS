/*
 * dselect - Debian package maintenance user interface
 * pkgdepcon.cc - dependency and conflict resolution
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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
packagelist::useavailable(pkginfo *pkg)
{
  if (pkg->clientdata &&
      pkg->clientdata->selected == PKG_WANT_INSTALL &&
      pkg_is_informative(pkg, &pkg->available) &&
      (!(pkg->status == PKG_STAT_INSTALLED ||
         pkg->status == PKG_STAT_TRIGGERSAWAITED ||
         pkg->status == PKG_STAT_TRIGGERSPENDING) ||
       dpkg_version_compare(&pkg->available.version,
                            &pkg->installed.version) > 0))
    return true;
  else
    return false;
}

pkgbin *
packagelist::find_pkgbin(pkginfo *pkg)
{
  pkgbin *pkgbin;

  pkgbin = useavailable(pkg) ? &pkg->available : &pkg->installed;
  debug(dbg_general, "packagelist[%p]::find_pkgbin(%s) useavailable=%d",
        this, pkgbin_name(pkg, pkgbin, pnaw_always), useavailable(pkg));

  return pkgbin;
}

int packagelist::checkdependers(pkginfo *pkg, int changemade) {
  struct deppossi *possi;

  for (possi = pkg->set->depended.available; possi; possi = possi->rev_next) {
    if (!useavailable(possi->up->up))
      continue;
    changemade = max(changemade, resolvedepcon(possi->up));
  }
  for (possi = pkg->set->depended.installed; possi; possi = possi->rev_next) {
    if (useavailable(possi->up->up))
      continue;
    changemade = max(changemade, resolvedepcon(possi->up));
  }
  return changemade;
}

int packagelist::resolvesuggest() {
  // We continually go around looking for things to change, but we may
  // only change the ‘suggested’ value if we also increase the ‘priority’
  // Return 2 if we made a change due to a Recommended, Depends or Conflicts,
  // or 1 if we offered or made a change because of an Optional line.
  debug(dbg_general, "packagelist[%p]::resolvesuggest()", this);
  int maxchangemade = 0;

  for (;;) {
    int changemade = 0;
    int index;

    for (index=0; index<nitems; index++) {
      if (!table[index]->pkg->set->name)
        continue;
      debug(dbg_depcon, "packagelist[%p]::resolvesuggest() loop[%i] %s / %d",
            this, index, pkg_name(table[index]->pkg, pnaw_always), changemade);
      dependency *depends;
      for (depends = find_pkgbin(table[index]->pkg)->depends;
           depends;
           depends= depends->next) {
        changemade = max(changemade, resolvedepcon(depends));
      }
      changemade= checkdependers(table[index]->pkg,changemade);
      for (depends = find_pkgbin(table[index]->pkg)->depends;
           depends;
           depends= depends->next) {
        if (depends->type != dep_provides) continue;
        changemade = checkdependers(&depends->list->ed->pkg, changemade);
      }
      debug(dbg_depcon, "packagelist[%p]::resolvesuggest() loop[%i] %s / -> %d",
            this, index, pkg_name(table[index]->pkg, pnaw_always), changemade);
    }
    if (!changemade) break;
    maxchangemade = max(maxchangemade, changemade);
  }
  debug(dbg_general, "packagelist[%p]::resolvesuggest() done; maxchangemade=%d",
        this, maxchangemade);
  return maxchangemade;
}

static bool
dep_update_best_to_change_stop(perpackagestate *& best, pkginfo *trythis)
{
  // There's no point trying to select a pure virtual package.
  if (!trythis->clientdata)
    return false;

  debug(dbg_depcon, "update_best_to_change(best=%s{%d}, test=%s{%d});",
        best ? pkg_name(best->pkg, pnaw_always) : "",
        best ? best->spriority : -1,
        trythis->set->name, trythis->clientdata->spriority);

  // If the problem is caused by us deselecting one of these packages
  // we should not try to select another one instead.
  if (trythis->clientdata->spriority == sp_deselecting)
    return true;

  // If we haven't found anything yet then this is our best so far.
  if (!best) goto yes;

  // If only one of the packages is available, use that one
  if (!pkg_is_informative(trythis, &trythis->available) &&
      pkg_is_informative(best->pkg, &best->pkg->available))
    return false;
  if (pkg_is_informative(trythis, &trythis->available) &&
      !pkg_is_informative(best->pkg, &best->pkg->available))
    goto yes;

  // Select the package with the lowest priority (i.e., the one of whom
  // we were least sure we wanted it deselected).
  if (trythis->clientdata->spriority > best->spriority)
    return false;
  if (trythis->clientdata->spriority < best->spriority) goto yes;

  // Pick the package with the must fundamental recommendation level.
  if (trythis->priority > best->pkg->priority)
    return false;
  if (trythis->priority < best->pkg->priority) goto yes;

  // If we're still unsure we'll change the first one in the list.
  return false;

 yes:
  debug(dbg_depcon, "update_best_to_change(); yes");

  best = trythis->clientdata;
  return false;
}

int
packagelist::deselect_one_of(pkginfo *per, pkginfo *ped, dependency *dep)
{
  perpackagestate *er= per->clientdata;
  perpackagestate *ed= ped->clientdata;

  if (!er || !would_like_to_install(er->selected,per) ||
      !ed || !would_like_to_install(ed->selected,ped)) return 0;

  add(dep, dp_must);

  er= per->clientdata;  // these can be changed by add
  ed= ped->clientdata;

  debug(dbg_depcon,
        "packagelist[%p]::deselect_one_of(): er %s{%d} ed %s{%d} [%p]",
        this, pkg_name(er->pkg, pnaw_always), er->spriority,
        pkg_name(ed->pkg, pnaw_always), ed->spriority, dep);

  perpackagestate *best;

  // Try not keep packages needing reinstallation.
  if (per->eflag & PKG_EFLAG_REINSTREQ)
    best = ed;
  else if (ped->eflag & PKG_EFLAG_REINSTREQ)
    best = er;
  // We'd rather change the one with the lowest priority.
  else if (er->spriority > ed->spriority)
    best = ed;
  else if (er->spriority < ed->spriority)
    best = er;
  // ... failing that the one with the highest priority.
  else if (er->pkg->priority < ed->pkg->priority)
    best = ed;
  else if (er->pkg->priority > ed->pkg->priority)
    best = er;
  // ... failing that, the second.
  else
    best = ed;

  debug(dbg_depcon, "packagelist[%p]::deselect_one_of(): best %s{%d}",
        this, pkg_name(best->pkg, pnaw_always), best->spriority);

  if (best->spriority >= sp_deselecting) return 0;
  best->suggested=
    best->pkg->status == PKG_STAT_NOTINSTALLED
      ? PKG_WANT_PURGE : PKG_WANT_DEINSTALL; // FIXME: configurable.
  best->selected= best->suggested;
  best->spriority= sp_deselecting;

  return 2;
}

int packagelist::resolvedepcon(dependency *depends) {
  perpackagestate *best, *fixbyupgrade;
  deppossi *possi, *provider;
  bool foundany;
  int rc;

  if (debug_has_flag(dbg_depcon)) {
    varbuf pkg_names;

    for (possi = depends->list; possi; possi = possi->next) {
      pkg_names += ' ';
      pkg_names += possi->ed->name;
    }

    debug(dbg_depcon,
          "packagelist[%p]::resolvedepcon([%p] %s --%s-->%s); (ing)->want=%s",
          this, depends, pkg_name(depends->up, pnaw_always),
          relatestrings[depends->type],
          pkg_names.str(), depends->up->clientdata ?
          wantstrings[depends->up->clientdata->suggested] : "(no clientdata)");
  }

  if (!depends->up->clientdata) return 0;

  switch (depends->type) {
  case dep_provides:
  case dep_replaces:
    return 0;

  case dep_enhances:
  case dep_suggests:
  case dep_recommends:
  case dep_depends:
  case dep_predepends:
    if (would_like_to_install(depends->up->clientdata->selected,depends->up) <= 0)
      return 0;

    fixbyupgrade = nullptr;

    possi = depends->list;
    while (possi && !deppossatisfied(possi, &fixbyupgrade))
      possi = possi->next;
    debug(dbg_depcon, "packagelist[%p]::resolvedepcon([%p]): depends found %s",
          this, depends, possi ? possi->ed->name : "[none]");
    if (possi) return 0;

    // Ensures all in the recursive list; adds info strings; ups priorities
    switch (depends->type) {
    case dep_enhances:
    case dep_suggests:
      rc = add(depends, dp_may);
      return rc;
    case dep_recommends:
      rc = add(depends, dp_should);
	break;
    default:
      rc = add(depends, dp_must);
    }

    if (fixbyupgrade) {
      debug(dbg_depcon,
            "packagelist[%p]::resolvedepcon([%p]): fixbyupgrade %s",
            this, depends, pkg_name(fixbyupgrade->pkg, pnaw_always));
      best= fixbyupgrade;
    } else {
      best = nullptr;
      for (possi= depends->list;
           possi;
           possi= possi->next) {
        foundany = false;
        if (possi->ed->pkg.clientdata)
          foundany = true;
        if (dep_update_best_to_change_stop(best, &possi->ed->pkg))
          goto mustdeselect;
        for (provider = possi->ed->depended.available;
             provider;
             provider = provider->rev_next) {
          if (provider->up->type != dep_provides) continue;
          if (provider->up->up->clientdata)
            foundany = true;
          if (dep_update_best_to_change_stop(best, provider->up->up)) goto mustdeselect;
        }
        if (!foundany) addunavailable(possi);
      }
      if (!best) {
        debug(dbg_depcon,
              "packagelist[%p]::resolvedepcon([%p]): mustdeselect nobest",
              this, depends);
        return rc;
      }
    }
    debug(dbg_depcon,
          "packagelist[%p]::resolvedepcon([%p]): select best=%s{%d}",
          this, depends, pkg_name(best->pkg, pnaw_always), best->spriority);
    if (best->spriority >= sp_selecting)
      return rc;
    /* Always select depends. Only select recommends if we got here because
     * of a manually-initiated install request. */
    if (depends->type != dep_recommends || manual_install) {
      best->selected = best->suggested = PKG_WANT_INSTALL;
      best->spriority= sp_selecting;
    }
    return rc ? 2 : 0;

  mustdeselect:
    best= depends->up->clientdata;
    debug(dbg_depcon,
          "packagelist[%p]::resolvedepcon([%p]): mustdeselect best=%s{%d}",
          this, depends, pkg_name(best->pkg, pnaw_always), best->spriority);

    if (best->spriority >= sp_deselecting)
      return rc;
    /* Always remove depends, but never remove recommends. */
    if (depends->type != dep_recommends) {
      best->selected= best->suggested=
        best->pkg->status == PKG_STAT_NOTINSTALLED
          ? PKG_WANT_PURGE : PKG_WANT_DEINSTALL; // FIXME: configurable
      best->spriority= sp_deselecting;
    }
    return rc ? 2 : 0;

  case dep_conflicts:
  case dep_breaks:
    debug(dbg_depcon, "packagelist[%p]::resolvedepcon([%p]): conflict",
          this, depends);

    if (would_like_to_install(depends->up->clientdata->selected,depends->up) == 0)
      return 0;

    debug(dbg_depcon,
          "packagelist[%p]::resolvedepcon([%p]): conflict installing 1",
          this, depends);

    if (!deppossatisfied(depends->list, nullptr))
      return 0;

    debug(dbg_depcon,
          "packagelist[%p]::resolvedepcon([%p]): conflict satisfied - ouch",
          this, depends);

    if (depends->up->set != depends->list->ed) {
      rc = deselect_one_of(depends->up, &depends->list->ed->pkg, depends);
      if (rc)
        return rc;
    }
    for (provider = depends->list->ed->depended.available;
         provider;
         provider = provider->rev_next) {
      if (provider->up->type != dep_provides) continue;
      if (provider->up->up == depends->up) continue; // conflicts & provides same thing
      rc = deselect_one_of(depends->up, provider->up->up, depends);
      if (rc)
        return rc;
    }
    debug(dbg_depcon, "packagelist[%p]::resolvedepcon([%p]): no desel",
          this, depends);
    return 0;

  default:
    internerr("unknown deptype %d", depends->type);
  }
  /* never reached, make gcc happy */
  return 1;
}

bool
packagelist::deppossatisfied(deppossi *possi, perpackagestate **fixbyupgrade)
{
  // ‘satisfied’ here for Conflicts and Breaks means that the
  //  restriction is violated, that is, that the target package is wanted
  int would;
  pkgwant want = PKG_WANT_PURGE;

  if (possi->ed->pkg.clientdata) {
    want = possi->ed->pkg.clientdata->selected;
    would = would_like_to_install(want, &possi->ed->pkg);
  } else {
    would= 0;
  }

  if ((possi->up->type == dep_conflicts || possi->up->type == dep_breaks)
      ? possi->up->up->set != possi->ed && would != 0
      : would > 0) {
    // If it's to be installed or left installed, then either it's of
    // the right version, and therefore OK, or a version must have
    // been specified, in which case we don't need to look at the rest
    // anyway.
    if (useavailable(&possi->ed->pkg)) {
      if (want != PKG_WANT_INSTALL)
        internerr("depossi package is not want-install, is %d", want);
      return versionsatisfied(&possi->ed->pkg.available, possi);
    } else {
      if (versionsatisfied(&possi->ed->pkg.installed, possi))
        return true;
      if (want == PKG_WANT_HOLD && fixbyupgrade && !*fixbyupgrade &&
          versionsatisfied(&possi->ed->pkg.available, possi) &&
          dpkg_version_compare(&possi->ed->pkg.available.version,
                               &possi->ed->pkg.installed.version) > 1)
        *fixbyupgrade = possi->ed->pkg.clientdata;
      return false;
    }
  }

  deppossi *provider;

  for (provider = possi->ed->depended.installed;
       provider;
       provider = provider->rev_next) {
    if (provider->up->type == dep_provides &&
        ((possi->up->type != dep_conflicts && possi->up->type != dep_breaks) ||
         provider->up->up->set != possi->up->up->set) &&
        provider->up->up->clientdata &&
        !useavailable(provider->up->up) &&
        would_like_to_install(provider->up->up->clientdata->selected,
                              provider->up->up) &&
        pkg_virtual_deppossi_satisfied(possi, provider))
      return true;
  }
  for (provider = possi->ed->depended.available;
       provider;
       provider = provider->rev_next) {
    if (provider->up->type != dep_provides ||
        ((possi->up->type == dep_conflicts || possi->up->type == dep_breaks) &&
         provider->up->up->set == possi->up->up->set) ||
        !provider->up->up->clientdata ||
        !would_like_to_install(provider->up->up->clientdata->selected,
                               provider->up->up) ||
        !pkg_virtual_deppossi_satisfied(possi, provider))
      continue;
    if (useavailable(provider->up->up))
      return true;
    if (fixbyupgrade && !*fixbyupgrade &&
        (!(provider->up->up->status == PKG_STAT_INSTALLED ||
           provider->up->up->status == PKG_STAT_TRIGGERSPENDING ||
           provider->up->up->status == PKG_STAT_TRIGGERSAWAITED) ||
         dpkg_version_compare(&provider->up->up->available.version,
                              &provider->up->up->installed.version) > 1))
      *fixbyupgrade = provider->up->up->clientdata;
  }
  return false;
}
