/*
 * dpkg - main program for package management
 * depcon.c - dependency and conflict checking
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006-2014 Guillem Jover <guillem@debian.org>
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
#include <stdlib.h>
#include <unistd.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/db-ctrl.h>
#include <dpkg/db-fsys.h>

#include "main.h"

struct deppossi_pkg_iterator {
  struct deppossi *possi;
  struct pkginfo *pkg_next;
  enum which_pkgbin which_pkgbin;
};

struct deppossi_pkg_iterator *
deppossi_pkg_iter_new(struct deppossi *possi, enum which_pkgbin wpb)
{
  struct deppossi_pkg_iterator *iter;

  iter = m_malloc(sizeof(*iter));
  iter->possi = possi;
  iter->pkg_next = &possi->ed->pkg;
  iter->which_pkgbin = wpb;

  return iter;
}

struct pkginfo *
deppossi_pkg_iter_next(struct deppossi_pkg_iterator *iter)
{
  struct pkginfo *pkg_cur;
  struct pkgbin *pkgbin;

  while ((pkg_cur = iter->pkg_next)) {
    iter->pkg_next = pkg_cur->arch_next;

    switch (iter->which_pkgbin) {
    case wpb_installed:
      pkgbin = &pkg_cur->installed;
      break;
    case wpb_available:
      pkgbin = &pkg_cur->available;
      break;
    case wpb_by_istobe:
      if (pkg_cur->clientdata &&
          pkg_cur->clientdata->istobe == PKG_ISTOBE_INSTALLNEW)
        pkgbin = &pkg_cur->available;
      else
        pkgbin = &pkg_cur->installed;
      break;
    default:
      internerr("unknown which_pkgbin %d", iter->which_pkgbin);
    }

    if (archsatisfied(pkgbin, iter->possi))
      return pkg_cur;
  }

  return NULL;
}

void
deppossi_pkg_iter_free(struct deppossi_pkg_iterator *iter)
{
  free(iter);
}

struct cyclesofarlink {
  struct cyclesofarlink *prev;
  struct pkginfo *pkg;
  struct deppossi *possi;
};

static bool findbreakcyclerecursive(struct pkginfo *pkg,
                                    struct cyclesofarlink *sofar);

static bool
foundcyclebroken(struct cyclesofarlink *thislink, struct cyclesofarlink *sofar,
                 struct pkginfo *dependedon, struct deppossi *possi)
{
  struct cyclesofarlink *sol;

  if(!possi)
    return false;

  /* We're investigating the dependency ‘possi’ to see if it
   * is part of a loop. To this end we look to see whether the
   * depended-on package is already one of the packages whose
   * dependencies we're searching. */
  for (sol = sofar; sol && sol->pkg != dependedon; sol = sol->prev);

  /* If not, we do a recursive search on it to see what we find. */
  if (!sol)
    return findbreakcyclerecursive(dependedon, thislink);

  debug(dbg_depcon,"found cycle");
  /* Right, we now break one of the links. We prefer to break
   * a dependency of a package without a postinst script, as
   * this is a null operation. If this is not possible we break
   * the other link in the recursive calling tree which mentions
   * this package (this being the first package involved in the
   * cycle). It doesn't particularly matter which we pick, but if
   * we break the earliest dependency we came across we may be
   * able to do something straight away when findbreakcycle returns. */
  sofar= thislink;
  for (sol = sofar; !(sol != sofar && sol->pkg == dependedon); sol = sol->prev) {
    if (!pkg_infodb_has_file(sol->pkg, &sol->pkg->installed, POSTINSTFILE))
      break;
  }

  /* Now we have either a package with no postinst, or the other
   * occurrence of the current package in the list. */
  sol->possi->cyclebreak = true;

  debug(dbg_depcon, "cycle broken at %s -> %s",
        pkg_name(sol->possi->up->up, pnaw_always), sol->possi->ed->name);

  return true;
}

/**
 * Cycle breaking works recursively down the package dependency tree.
 *
 * ‘sofar’ is the list of packages we've descended down already - if we
 * encounter any of its packages again in a dependency we have found a cycle.
 */
static bool
findbreakcyclerecursive(struct pkginfo *pkg, struct cyclesofarlink *sofar)
{
  struct cyclesofarlink thislink, *sol;
  struct dependency *dep;
  struct deppossi *possi, *providelink;
  struct pkginfo *provider, *pkg_pos;

  if (pkg->clientdata->color == PKG_CYCLE_BLACK)
    return false;
  pkg->clientdata->color = PKG_CYCLE_GRAY;

  if (debug_has_flag(dbg_depcondetail)) {
    struct varbuf str_pkgs = VARBUF_INIT;

    for (sol = sofar; sol; sol = sol->prev) {
      varbuf_add_str(&str_pkgs, " <- ");
      varbuf_add_pkgbin_name(&str_pkgs, sol->pkg, &sol->pkg->installed, pnaw_nonambig);
    }
    debug(dbg_depcondetail, "findbreakcyclerecursive %s %s",
          pkg_name(pkg, pnaw_always), varbuf_str(&str_pkgs));
    varbuf_destroy(&str_pkgs);
  }
  thislink.pkg= pkg;
  thislink.prev = sofar;
  thislink.possi = NULL;
  for (dep= pkg->installed.depends; dep; dep= dep->next) {
    if (dep->type != dep_depends && dep->type != dep_predepends) continue;
    for (possi= dep->list; possi; possi= possi->next) {
      struct deppossi_pkg_iterator *possi_iter;

      /* Don't find the same cycles again. */
      if (possi->cyclebreak) continue;
      thislink.possi= possi;

      possi_iter = deppossi_pkg_iter_new(possi, wpb_installed);
      while ((pkg_pos = deppossi_pkg_iter_next(possi_iter)))
        if (foundcyclebroken(&thislink, sofar, pkg_pos, possi)) {
          deppossi_pkg_iter_free(possi_iter);
          return true;
        }
      deppossi_pkg_iter_free(possi_iter);

      /* Right, now we try all the providers ... */
      for (providelink = possi->ed->depended.installed;
           providelink;
           providelink = providelink->rev_next) {
        if (providelink->up->type != dep_provides) continue;
        provider= providelink->up->up;
        if (provider->clientdata->istobe == PKG_ISTOBE_NORMAL)
          continue;
        /* We don't break things at ‘provides’ links, so ‘possi’ is
         * still the one we use. */
        if (foundcyclebroken(&thislink, sofar, provider, possi))
          return true;
      }
    }
  }
  /* Nope, we didn't find a cycle to break. */
  pkg->clientdata->color = PKG_CYCLE_BLACK;
  return false;
}

bool
findbreakcycle(struct pkginfo *pkg)
{
  struct pkg_hash_iter *iter;
  struct pkginfo *tpkg;

  /* Clear the visited flag of all packages before we traverse them. */
  iter = pkg_hash_iter_new();
  while ((tpkg = pkg_hash_iter_next_pkg(iter))) {
    ensure_package_clientdata(tpkg);
    tpkg->clientdata->color = PKG_CYCLE_WHITE;
  }
  pkg_hash_iter_free(iter);

  return findbreakcyclerecursive(pkg, NULL);
}

void describedepcon(struct varbuf *addto, struct dependency *dep) {
  struct varbuf depstr = VARBUF_INIT;

  varbufdependency(&depstr, dep);

  switch (dep->type) {
  case dep_depends:
    varbuf_add_fmt(addto, _("%s depends on %s"),
                   pkg_name(dep->up, pnaw_nonambig), depstr.buf);
    break;
  case dep_predepends:
    varbuf_add_fmt(addto, _("%s pre-depends on %s"),
                   pkg_name(dep->up, pnaw_nonambig), depstr.buf);
    break;
  case dep_recommends:
    varbuf_add_fmt(addto, _("%s recommends %s"),
                   pkg_name(dep->up, pnaw_nonambig), depstr.buf);
    break;
  case dep_suggests:
    varbuf_add_fmt(addto, _("%s suggests %s"),
                   pkg_name(dep->up, pnaw_nonambig), depstr.buf);
    break;
  case dep_breaks:
    varbuf_add_fmt(addto, _("%s breaks %s"),
                   pkg_name(dep->up, pnaw_nonambig), depstr.buf);
    break;
  case dep_conflicts:
    varbuf_add_fmt(addto, _("%s conflicts with %s"),
                   pkg_name(dep->up, pnaw_nonambig), depstr.buf);
    break;
  case dep_enhances:
    varbuf_add_fmt(addto, _("%s enhances %s"),
                   pkg_name(dep->up, pnaw_nonambig), depstr.buf);
    break;
  default:
    internerr("unknown deptype '%d'", dep->type);
  }

  varbuf_destroy(&depstr);
}

/*
 * *whynot must already have been initialized; it need not be
 * empty though - it will be reset before use.
 *
 * If depisok returns false for ‘not OK’ it will contain a description,
 * newline and NUL terminated, of the reason.
 *
 * If depisok returns true it will contain garbage.
 * allowunconfigd should be non-zero during the ‘Pre-Depends’ checking
 * before a package is unpacked, when it is sufficient for the package
 * to be unpacked provided that both the unpacked and previously-configured
 * versions are acceptable.
 *
 * On false return (‘not OK’), *canfixbyremove refers to a package which
 * if removed (dep_conflicts) or deconfigured (dep_breaks) will fix
 * the problem. Caller may pass NULL for canfixbyremove and need not
 * initialize *canfixbyremove.
 *
 * On false return (‘not OK’), *canfixbytrigaw refers to a package which
 * can fix the problem if all the packages listed in Triggers-Awaited have
 * their triggers processed. Caller may pass NULL for canfixbytrigaw and
 * need not initialize *canfixbytrigaw.
 */
bool
depisok(struct dependency *dep, struct varbuf *whynot,
        struct pkginfo **canfixbyremove, struct pkginfo **canfixbytrigaw,
        bool allowunconfigd)
{
  struct deppossi *possi;
  struct deppossi *provider;
  struct pkginfo *pkg_pos;

  if (dep->type != dep_depends &&
      dep->type != dep_predepends &&
      dep->type != dep_breaks &&
      dep->type != dep_conflicts &&
      dep->type != dep_recommends &&
      dep->type != dep_suggests &&
      dep->type != dep_enhances)
    internerr("unknown dependency type %d", dep->type);

  if (canfixbyremove)
    *canfixbyremove = NULL;
  if (canfixbytrigaw)
    *canfixbytrigaw = NULL;

  /* The dependency is always OK if we're trying to remove the depend*ing*
   * package. */
  switch (dep->up->clientdata->istobe) {
  case PKG_ISTOBE_REMOVE:
  case PKG_ISTOBE_DECONFIGURE:
    return true;
  case PKG_ISTOBE_NORMAL:
    /* Only installed packages can be made dependency problems. */
    switch (dep->up->status) {
    case PKG_STAT_INSTALLED:
    case PKG_STAT_TRIGGERSPENDING:
    case PKG_STAT_TRIGGERSAWAITED:
      break;
    case PKG_STAT_HALFCONFIGURED:
    case PKG_STAT_UNPACKED:
    case PKG_STAT_HALFINSTALLED:
      if (dep->type == dep_predepends ||
          dep->type == dep_conflicts ||
          dep->type == dep_breaks)
        break;
      /* Fall through. */
    case PKG_STAT_CONFIGFILES:
    case PKG_STAT_NOTINSTALLED:
      return true;
    default:
      internerr("unknown status depending '%d'", dep->up->status);
    }
    break;
  case PKG_ISTOBE_INSTALLNEW:
  case PKG_ISTOBE_PREINSTALL:
    break;
  default:
    internerr("unknown istobe depending '%d'", dep->up->clientdata->istobe);
  }

  /* Describe the dependency, in case we have to moan about it. */
  varbuf_reset(whynot);
  varbuf_add_char(whynot, ' ');
  describedepcon(whynot, dep);
  varbuf_add_char(whynot, '\n');

  /* TODO: Check dep_enhances as well. */
  if (dep->type == dep_depends || dep->type == dep_predepends ||
      dep->type == dep_recommends || dep->type == dep_suggests ) {
    struct varbuf linebuf = VARBUF_INIT;

    /* Go through the alternatives. As soon as we find one that
     * we like, we return ‘true’ straight away. Otherwise, when we get to
     * the end we'll have accumulated all the reasons in whynot and
     * can return ‘false’. */

    for (possi= dep->list; possi; possi= possi->next) {
      struct deppossi_pkg_iterator *possi_iter;

      possi_iter = deppossi_pkg_iter_new(possi, wpb_by_istobe);
      while ((pkg_pos = deppossi_pkg_iter_next(possi_iter))) {
        switch (pkg_pos->clientdata->istobe) {
        case PKG_ISTOBE_REMOVE:
          varbuf_set_fmt(&linebuf, _("  %.250s is to be removed.\n"),
                         pkg_name(pkg_pos, pnaw_nonambig));
          break;
        case PKG_ISTOBE_DECONFIGURE:
          varbuf_set_fmt(&linebuf, _("  %.250s is to be deconfigured.\n"),
                         pkg_name(pkg_pos, pnaw_nonambig));
          break;
        case PKG_ISTOBE_INSTALLNEW:
          if (versionsatisfied(&pkg_pos->available, possi)) {
            deppossi_pkg_iter_free(possi_iter);
            varbuf_destroy(&linebuf);
            return true;
          }
          varbuf_set_fmt(&linebuf, _("  %.250s is to be installed, "
                                     "but is version %.250s.\n"),
                         pkgbin_name(pkg_pos, &pkg_pos->available, pnaw_nonambig),
                         versiondescribe(&pkg_pos->available.version, vdew_nonambig));
          break;
        case PKG_ISTOBE_NORMAL:
        case PKG_ISTOBE_PREINSTALL:
          switch (pkg_pos->status) {
          case PKG_STAT_INSTALLED:
          case PKG_STAT_TRIGGERSPENDING:
            if (versionsatisfied(&pkg_pos->installed, possi)) {
              deppossi_pkg_iter_free(possi_iter);
              varbuf_destroy(&linebuf);
              return true;
            }
            varbuf_set_fmt(&linebuf, _("  %.250s is installed, "
                                       "but is version %.250s.\n"),
                           pkg_name(pkg_pos, pnaw_nonambig),
                           versiondescribe(&pkg_pos->installed.version, vdew_nonambig));
            break;
          case PKG_STAT_NOTINSTALLED:
            /* Don't say anything about this yet - it might be a virtual package.
             * Later on, if nothing has put anything in linebuf, we know that it
             * isn't and issue a diagnostic then. */
            varbuf_reset(&linebuf);
            break;
          case PKG_STAT_TRIGGERSAWAITED:
              if (canfixbytrigaw && versionsatisfied(&pkg_pos->installed, possi))
                *canfixbytrigaw = pkg_pos;
              /* Fall through. */
          case PKG_STAT_UNPACKED:
          case PKG_STAT_HALFCONFIGURED:
            if (allowunconfigd) {
              if (!dpkg_version_is_informative(&pkg_pos->configversion)) {
                varbuf_set_fmt(&linebuf, _("  %.250s is unpacked, "
                                           "but has never been configured.\n"),
                               pkg_name(pkg_pos, pnaw_nonambig));
                break;
              } else if (!versionsatisfied(&pkg_pos->installed, possi)) {
                varbuf_set_fmt(&linebuf, _("  %.250s is unpacked, "
                                           "but is version %.250s.\n"),
                               pkg_name(pkg_pos, pnaw_nonambig),
                               versiondescribe(&pkg_pos->installed.version,
                                               vdew_nonambig));
                break;
              } else if (!dpkg_version_relate(&pkg_pos->configversion,
                                              possi->verrel,
                                              &possi->version)) {
                varbuf_set_fmt(&linebuf, _("  %.250s latest configured "
                                           "version is %.250s.\n"),
                                pkg_name(pkg_pos, pnaw_nonambig),
                                versiondescribe(&pkg_pos->configversion,
                                                vdew_nonambig));
                break;
              } else {
                deppossi_pkg_iter_free(possi_iter);
                varbuf_destroy(&linebuf);
                return true;
              }
            }
            /* Fall through. */
          default:
            varbuf_set_fmt(&linebuf, _("  %.250s is %s.\n"),
                           pkg_name(pkg_pos, pnaw_nonambig),
                           gettext(statusstrings[pkg_pos->status]));
            break;
          }
          break;
        default:
          internerr("unknown istobe depended '%d'", pkg_pos->clientdata->istobe);
        }
        varbuf_add_varbuf(whynot, &linebuf);
      }
      deppossi_pkg_iter_free(possi_iter);

        /* See if the package we're about to install Provides it. */
        for (provider = possi->ed->depended.available;
             provider;
             provider = provider->rev_next) {
          if (provider->up->type != dep_provides) continue;
          if (!pkg_virtual_deppossi_satisfied(possi, provider))
            continue;
          if (provider->up->up->clientdata->istobe == PKG_ISTOBE_INSTALLNEW) {
            varbuf_destroy(&linebuf);
            return true;
          }
        }

        /* Now look at the packages already on the system. */
        for (provider = possi->ed->depended.installed;
             provider;
             provider = provider->rev_next) {
          if (provider->up->type != dep_provides) continue;
          if (!pkg_virtual_deppossi_satisfied(possi, provider))
            continue;

          switch (provider->up->up->clientdata->istobe) {
          case PKG_ISTOBE_INSTALLNEW:
            /* Don't pay any attention to the Provides field of the
             * currently-installed version of the package we're trying
             * to install. We dealt with that by using the available
             * information above. */
            continue;
          case PKG_ISTOBE_REMOVE:
            varbuf_set_fmt(&linebuf, _("  %.250s provides %.250s "
                                       "but is to be removed.\n"),
                           pkg_name(provider->up->up, pnaw_nonambig),
                            possi->ed->name);
            break;
          case PKG_ISTOBE_DECONFIGURE:
            varbuf_set_fmt(&linebuf, _("  %.250s provides %.250s "
                                       "but is to be deconfigured.\n"),
                           pkg_name(provider->up->up, pnaw_nonambig),
                           possi->ed->name);
            break;
          case PKG_ISTOBE_NORMAL:
          case PKG_ISTOBE_PREINSTALL:
            if (provider->up->up->status == PKG_STAT_INSTALLED ||
                provider->up->up->status == PKG_STAT_TRIGGERSPENDING) {
              varbuf_destroy(&linebuf);
              return true;
            }
            if (provider->up->up->status == PKG_STAT_TRIGGERSAWAITED)
              *canfixbytrigaw = provider->up->up;
            varbuf_set_fmt(&linebuf, _("  %.250s provides %.250s but is %s.\n"),
                           pkg_name(provider->up->up, pnaw_nonambig),
                           possi->ed->name,
                           gettext(statusstrings[provider->up->up->status]));
            break;
          default:
            internerr("unknown istobe provider '%d'",
                      provider->up->up->clientdata->istobe);
          }
          varbuf_add_varbuf(whynot, &linebuf);
        }

        if (linebuf.used == 0) {
          /* If the package wasn't installed at all, and we haven't said
           * yet why this isn't satisfied, we should say so now. */
          varbuf_set_fmt(&linebuf, _("  %.250s is not installed.\n"),
                         possi->ed->name);
          varbuf_add_varbuf(whynot, &linebuf);
        }
    }
    varbuf_destroy(&linebuf);

    return false;
  } else {
    int nconflicts;

    /* It's conflicts or breaks. There's only one main alternative,
     * but we also have to consider Providers. We return ‘false’ as soon
     * as we find something that matches the conflict, and only describe
     * it then. If we get to the end without finding anything we return
     * ‘true’. */

    possi= dep->list;
    nconflicts= 0;

    if (possi->ed != possi->up->up->set) {
      struct deppossi_pkg_iterator *possi_iter;

      /* If the package conflicts with or breaks itself it must mean
       * other packages which provide the same virtual name. We
       * therefore don't look at the real package and go on to the
       * virtual ones. */

      possi_iter = deppossi_pkg_iter_new(possi, wpb_by_istobe);
      while ((pkg_pos = deppossi_pkg_iter_next(possi_iter))) {
        switch (pkg_pos->clientdata->istobe) {
        case PKG_ISTOBE_REMOVE:
          break;
        case PKG_ISTOBE_INSTALLNEW:
          if (!versionsatisfied(&pkg_pos->available, possi))
            break;
          varbuf_add_fmt(whynot, _("  %.250s (version %.250s) is "
                                   "to be installed.\n"),
                         pkgbin_name(pkg_pos, &pkg_pos->available,
                                     pnaw_nonambig),
                         versiondescribe(&pkg_pos->available.version,
                                         vdew_nonambig));
          if (!canfixbyremove) {
            deppossi_pkg_iter_free(possi_iter);
            return false;
          }
          nconflicts++;
          *canfixbyremove = pkg_pos;
          break;
        case PKG_ISTOBE_DECONFIGURE:
          if (dep->type == dep_breaks)
            break; /* Already deconfiguring this. */
          /* Fall through. */
        case PKG_ISTOBE_NORMAL:
        case PKG_ISTOBE_PREINSTALL:
          switch (pkg_pos->status) {
          case PKG_STAT_NOTINSTALLED:
          case PKG_STAT_CONFIGFILES:
            break;
          case PKG_STAT_HALFINSTALLED:
          case PKG_STAT_UNPACKED:
          case PKG_STAT_HALFCONFIGURED:
            if (dep->type == dep_breaks)
              break; /* No problem. */
            /* Fall through. */
          case PKG_STAT_INSTALLED:
          case PKG_STAT_TRIGGERSPENDING:
          case PKG_STAT_TRIGGERSAWAITED:
            if (!versionsatisfied(&pkg_pos->installed, possi))
              break;
            varbuf_add_fmt(whynot, _("  %.250s (version %.250s) is "
                                     "present and %s.\n"),
                           pkg_name(pkg_pos, pnaw_nonambig),
                           versiondescribe(&pkg_pos->installed.version,
                                           vdew_nonambig),
                           gettext(statusstrings[pkg_pos->status]));
            if (!canfixbyremove) {
              deppossi_pkg_iter_free(possi_iter);
              return false;
            }
            nconflicts++;
            *canfixbyremove = pkg_pos;
          }
          break;
        default:
          internerr("unknown istobe conflict '%d'", pkg_pos->clientdata->istobe);
        }
      }
      deppossi_pkg_iter_free(possi_iter);
    }

      /* See if the package we're about to install Provides it. */
      for (provider = possi->ed->depended.available;
           provider;
           provider = provider->rev_next) {
        if (provider->up->type != dep_provides) continue;
        if (provider->up->up->clientdata->istobe != PKG_ISTOBE_INSTALLNEW)
          continue;
        if (provider->up->up->set == dep->up->set)
          continue; /* Conflicts and provides the same. */
        if (!pkg_virtual_deppossi_satisfied(possi, provider))
          continue;
        varbuf_add_fmt(whynot, _("  %.250s provides %.250s and is "
                                 "to be installed.\n"),
                       pkgbin_name(provider->up->up, &provider->up->up->available,
                                   pnaw_nonambig), possi->ed->name);
        /* We can't remove the one we're about to install: */
        if (canfixbyremove)
          *canfixbyremove = NULL;
        return false;
      }

      /* Now look at the packages already on the system. */
      for (provider = possi->ed->depended.installed;
           provider;
           provider = provider->rev_next) {
        if (provider->up->type != dep_provides) continue;

        if (provider->up->up->set == dep->up->set)
          continue; /* Conflicts and provides the same. */

        if (!pkg_virtual_deppossi_satisfied(possi, provider))
          continue;

        switch (provider->up->up->clientdata->istobe) {
        case PKG_ISTOBE_INSTALLNEW:
          /* Don't pay any attention to the Provides field of the
           * currently-installed version of the package we're trying
           * to install. We dealt with that package by using the
           * available information above. */
          continue;
        case PKG_ISTOBE_REMOVE:
          continue;
        case PKG_ISTOBE_DECONFIGURE:
          if (dep->type == dep_breaks)
            continue; /* Already deconfiguring. */
          /* Fall through. */
        case PKG_ISTOBE_NORMAL:
        case PKG_ISTOBE_PREINSTALL:
          switch (provider->up->up->status) {
          case PKG_STAT_NOTINSTALLED:
          case PKG_STAT_CONFIGFILES:
            continue;
          case PKG_STAT_HALFINSTALLED:
          case PKG_STAT_UNPACKED:
          case PKG_STAT_HALFCONFIGURED:
            if (dep->type == dep_breaks)
              break; /* No problem. */
            /* Fall through. */
          case PKG_STAT_INSTALLED:
          case PKG_STAT_TRIGGERSPENDING:
          case PKG_STAT_TRIGGERSAWAITED:
            varbuf_add_fmt(whynot, _("  %.250s provides %.250s and "
                                     "is present and %s.\n"),
                           pkg_name(provider->up->up, pnaw_nonambig),
                           possi->ed->name,
                           gettext(statusstrings[provider->up->up->status]));
            if (!canfixbyremove)
              return false;
            nconflicts++;
            *canfixbyremove= provider->up->up;
            break;
          }
          break;
        default:
          internerr("unknown istobe conflict provider '%d'",
                    provider->up->up->clientdata->istobe);
        }
      }

    if (!nconflicts)
      return true;
    if (nconflicts > 1)
      *canfixbyremove = NULL;
    return false;

  } /* if (dependency) {...} else {...} */
}
