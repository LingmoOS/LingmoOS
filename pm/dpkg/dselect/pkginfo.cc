/*
 * dselect - Debian package maintenance user interface
 * pkginfo.cc - handles (re)draw of package list window infopad
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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
#include <dpkg/string.h>

#include "dselect.h"
#include "bindings.h"
#include "helpmsgs.h"

const struct helpmenuentry *packagelist::helpmenulist() {
  static const struct helpmenuentry
    rw[]= {
      { 'i', &hlp_mainintro       },
      { 'k', &hlp_listkeys        },
      { 'l', &hlp_displayexplain1 },
      { 'd', &hlp_displayexplain2 },
      {  0                        }
    },
    ro[]= {
      { 'i', &hlp_readonlyintro   },
      { 'k', &hlp_listkeys        },
      { 'l', &hlp_displayexplain1 },
      { 'd', &hlp_displayexplain2 },
      {  0                        }
    },
    recur[]= {
      { 'i', &hlp_recurintro      },
      { 'k', &hlp_listkeys        },
      { 'l', &hlp_displayexplain1 },
      { 'd', &hlp_displayexplain2 },
      {  0                        }
    };
  return
    modstatdb_get_status() == msdbrw_readonly ? ro :
    !recursive ? rw :
                 recur;
}

bool
packagelist::itr_recursive()
{
  return recursive;
}

const packagelist::infotype packagelist::infoinfos[]= {
  { &packagelist::itr_recursive, &packagelist::itd_relations         },
  { nullptr,                     &packagelist::itd_description       },
  { nullptr,                     &packagelist::itd_statuscontrol     },
  { nullptr,                     &packagelist::itd_availablecontrol  },
  { nullptr,                     nullptr                             }
};

const packagelist::infotype *const packagelist::baseinfo= infoinfos;

void packagelist::severalinfoblurb()
{
  varbuf vb(_("The line you have highlighted represents many packages; "
     "if you ask to install, remove, hold, etc. it you will affect all "
     "the packages which match the criterion shown.\n"
     "\n"
     "If you move the highlight to a line for a particular package "
     "you will see information about that package displayed here."
     "\n"
     "You can use 'o' and 'O' to change the sort order and give yourself "
     "the opportunity to mark packages in different kinds of groups."));
  wordwrapinfo(0, vb.str());
}

void packagelist::itd_relations() {
  whatinfovb += _("Interrelationships");

  if (table[cursorline]->pkg->set->name) {
    debug(dbg_general, "packagelist[%p]::idt_relations(); '%s'",
          this, table[cursorline]->relations.str());
    waddstr(infopad, table[cursorline]->relations.str());
  } else {
    severalinfoblurb();
  }
}

void packagelist::itd_description() {
  whatinfovb += _("Description");

  if (table[cursorline]->pkg->set->name) {
    const char *m= table[cursorline]->pkg->available.description;
    if (str_is_unset(m))
      m = table[cursorline]->pkg->installed.description;
    if (str_is_unset(m))
      m = _("No description available.");
    const char *p = strchrnul(m, '\n');
    ptrdiff_t l = p - m;

    wattrset(infopad, part_attr[info_head]);
    waddstr(infopad, table[cursorline]->pkg->set->name);
    waddstr(infopad," - ");
    waddnstr(infopad,m,l);
    wattrset(infopad, part_attr[info_body]);
    if (*p) {
      waddstr(infopad,"\n\n");
      wordwrapinfo(1,++p);
    }
  } else {
    severalinfoblurb();
  }
}

void packagelist::itd_statuscontrol() {
  whatinfovb += _("Installed control file information");

  werase(infopad);
  if (!table[cursorline]->pkg->set->name) {
    severalinfoblurb();
  } else {
    varbuf vb;
    varbuf_stanza(&vb, table[cursorline]->pkg,
                  &table[cursorline]->pkg->installed);
    debug(dbg_general, "packagelist[%p]::idt_statuscontrol(); '%s'",
          this, vb.str());
    waddstr(infopad, vb.str());
  }
}

void packagelist::itd_availablecontrol() {
  whatinfovb += _("Available control file information");

  werase(infopad);
  if (!table[cursorline]->pkg->set->name) {
    severalinfoblurb();
  } else {
    varbuf vb;
    varbuf_stanza(&vb, table[cursorline]->pkg,
                  &table[cursorline]->pkg->available);
    debug(dbg_general, "packagelist[%p]::idt_availablecontrol(); '%s'",
          this, vb.str());
    waddstr(infopad, vb.str());
  }
}

void packagelist::redrawinfo() {
  for (;;) {
    if (!currentinfo || !currentinfo->display) currentinfo= baseinfo;
    if (!currentinfo->relevant) break;
    if ((this->*currentinfo->relevant)()) break;
    currentinfo++;
  }
  if (!info_height) return;
  whatinfovb.reset();
  werase(infopad); wmove(infopad,0,0);

  debug(dbg_general, "packagelist[%p]::redrawinfo(); #=%zd",
        this, currentinfo - baseinfo);

  (this->*currentinfo->display)();

  int y,x;
  getyx(infopad, y,x);
  if (x) y++;
  infolines= y;

  refreshinfo();
}
