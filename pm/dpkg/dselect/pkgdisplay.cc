/*
 * dselect - Debian package maintenance user interface
 * pkgdisplay.cc - package list display
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006, 2008-2015 Guillem Jover <guillem@debian.org>
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
#include "pkglist.h"

/* These MUST be in the same order as the corresponding enums in dpkg-db.h */
const char
  *const wantstrings[]=   { N_("new package"),
			    N_("install"),
			    N_("hold"),
			    N_("remove"),
			    N_("purge"),
			    nullptr },

  /* TRANSLATORS: The space is a trick to work around gettext which uses
   * the empty string to store information about the translation. DO NOT
   * CHANGE THAT IN A TRANSLATION! The code really relies on that being
   * a single space. */
  *const eflagstrings[]=   { N_(" "),
			     N_("REINSTALL"),
			     nullptr },

  *const statusstrings[]= { N_("not installed"),
			    N_("removed (configs remain)"),
			    N_("half installed"),
			    N_("unpacked (not set up)"),
			    N_("half configured (config failed)"),
			    N_("awaiting trigger processing"),
			    N_("triggered"),
			    N_("installed"),
			    nullptr },

  *const prioritystrings[]=  { N_("Required"),
			       N_("Important"),
			       N_("Standard"),
			       N_("Optional"),
			       N_("Extra"),
			       N_("Unclassified"),
			       nullptr },

  *const relatestrings[]= { N_("suggests"),
			    N_("recommends"),
			    N_("depends on"),
			    N_("pre-depends on"),
			    N_("breaks"),
			    N_("conflicts with"),
			    N_("provides"),
			    N_("replaces"),
			    N_("enhances"),
			    nullptr },

  *const priorityabbrevs[]=  { N_("Req"),
			       N_("Imp"),
			       N_("Std"),
			       N_("Opt"),
			       N_("Xtr"),
			       N_("bUG"),
			       N_("?") };

const char statuschars[] = " -IUCWt*";
const char eflagchars[] = " R";
const char wantchars[]=     "n*=-_";

/* These MUST be in the same order as the corresponding enums in pkglist.h */
const char
  *const ssaabbrevs[]= { N_("Broken"),
                         N_("New"),
                         N_("Upgradable"),
                         N_("Obsolete/local"),
                         N_("Installed"),
                         N_("Available"),
                         N_("Removed") },
  *const ssastrings[]= { N_("Brokenly installed packages"),
                         N_("Newly available packages"),
                         N_("Upgradable packages"),
                         N_("Obsolete and locally created packages"),
                         N_("Installed packages"),
                         N_("Available not installed packages"),
                         N_("Removed and no longer available packages") };

const char
  *const sssstrings[]= { N_("Brokenly installed packages"),
                         N_("Installed packages"),
                         N_("Removed packages (configuration still present)"),
                         N_("Purged packages and those never installed") },
  *const sssabbrevs[]= { N_("Broken"),
                         N_("Installed"),
                         N_("Removed"),
                         N_("Purged") };

static int maximumstring(const char *const *array) {
  int maxlen= 0;
  while (*array) {
    int l= strlen(gettext(*array));
    const char *p= strchr(*array, '(');
    if (p && p > *array && *--p == ' ') l= p - *array;
    if (l > maxlen) maxlen= l;
    array++;
  }
  return maxlen;
}

void packagelist::setwidths() {
  debug(dbg_general, "packagelist[%p]::setwidths()", this);

  col_cur_x = 0;

  if (verbose) {
    add_column(col_status_hold, _("Error"), 9);
    add_column(col_status_status, _("Installed?"), maximumstring(statusstrings));
    add_column(col_status_old_want, _("Old mark"), maximumstring(wantstrings));
    add_column(col_status_new_want, _("Marked for"), maximumstring(wantstrings));
  } else {
    add_column(col_status, _("EIOM"), 4);
  }

  if (sortorder == so_section)
    add_column(col_section, _("Section"), 8);
  add_column(col_priority, _("Priority"), verbose ? 8 : 3);
  if (sortorder != so_section)
    add_column(col_section, _("Section"), 8);

  add_column(col_package, _("Package"), verbose ? 16 : 12);

  switch (archdisplayopt) {
  case ado_none:
    col_archinstalled.blank();
    col_archavailable.blank();
    break;
  case ado_available:
    col_archinstalled.blank();
    add_column(col_archavailable, _("Avail.arch"), verbose ? 14 : 10);
    break;
  case ado_both:
    add_column(col_archinstalled, _("Inst.arch"), verbose ? 14 : 10);
    add_column(col_archavailable, _("Avail.arch"), verbose ? 14 : 10);
    break;
  default:
    internerr("unknown archdisplayopt %d", archdisplayopt);
  }

  switch (versiondisplayopt) {
  case vdo_none:
    col_versioninstalled.blank();
    col_versionavailable.blank();
    break;
  case vdo_available:
    col_versioninstalled.blank();
    add_column(col_versionavailable, _("Avail.ver"), 11);
    break;
  case vdo_both:
    add_column(col_versioninstalled, _("Inst.ver"), 11);
    add_column(col_versionavailable, _("Avail.ver"), 11);
    break;
  default:
    internerr("unknown versiondisplayopt %d", versiondisplayopt);
  }

  end_column(col_description, _("Description"));
}

void packagelist::redrawtitle() {
  if (title_height) {
    int x, y DPKG_ATTR_UNUSED;

    mywerase(titlewin);
    mvwaddnstr(titlewin,0,0,
               recursive ?  _("dselect - recursive package listing") :
               modstatdb_get_status() == msdbrw_readonly ?
                            _("dselect - inspection of package states") :
                            _("dselect - main package listing"),
               xmax);
    getyx(titlewin,y,x);
    if (x < xmax) {
      switch (sortorder) {
      case so_section:
        switch (statsortorder) {
        case sso_unsorted:
          waddnstr(titlewin, _(" (by section)"), xmax-x);
          break;
        case sso_avail:
          waddnstr(titlewin, _(" (avail., section)"), xmax-x);
          break;
        case sso_state:
          waddnstr(titlewin, _(" (status, section)"), xmax-x);
          break;
        default:
          internerr("bad statsort %d on so_section", statsortorder);
        }
        break;
      case so_priority:
        switch (statsortorder) {
        case sso_unsorted:
          waddnstr(titlewin, _(" (by priority)"), xmax-x);
          break;
        case sso_avail:
          waddnstr(titlewin, _(" (avail., priority)"), xmax-x);
          break;
        case sso_state:
          waddnstr(titlewin, _(" (status, priority)"), xmax-x);
          break;
        default:
          internerr("bad statsort %d on so_priority", statsortorder);
        }
        break;
      case so_alpha:
        switch (statsortorder) {
        case sso_unsorted:
          waddnstr(titlewin, _(" (alphabetically)"), xmax-x);
          break;
        case sso_avail:
          waddnstr(titlewin, _(" (by availability)"), xmax-x);
          break;
        case sso_state:
          waddnstr(titlewin, _(" (by status)"), xmax-x);
          break;
        default:
          internerr("bad statsort %d on so_priority", statsortorder);
        }
        break;
      case so_unsorted:
        break;
      default:
        internerr("bad sort %d", sortorder);
      }
    }
    const char *helpstring;

    if (modstatdb_get_status() == msdbrw_write)
      helpstring = (verbose ? _(" mark:+/=/- terse:v help:?")
                            : _(" mark:+/=/- verbose:v help:?"));
    else
      helpstring = (verbose ? _(" terse:v help:?")
                            : _(" verbose:v help:?"));

    int l= strlen(helpstring);
    getyx(titlewin,y,x);
    if (xmax-l > 0) {
      mvwaddstr(titlewin,0,xmax-l, helpstring);
    }
    wnoutrefresh(titlewin);
  }
}
