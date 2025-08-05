/*
 * dselect - Debian package maintenance user interface
 * bindings.cc - keybinding manager object definitions and default bindings
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

#include "dselect.h"
#include "bindings.h"

keybindings::keybindings(const interpretation *ints, const orgbinding *orgbindings) {
  interps= ints;
  bindings = nullptr;
  const orgbinding *b= orgbindings;
  while (b->action) { bind(b->key,b->action); b++; }
  describestart();
}

bool
keybindings::bind(int key, const char *action)
{
  if (key == -1)
    return false;

  const interpretation *interp = interps;
  while (interp->action && strcmp(interp->action, action))
    interp++;
  if (!interp->action)
    return false;

  const description *desc = descriptions;
  while (desc->action && strcmp(desc->action, action))
   desc++;

  binding *b = bindings;
  while (b && b->key != key)
    b = b->next;

  if (!b) {
    b = new binding;
    b->key = key;
    b->next = bindings;
    bindings = b;
  }
  b->interp = interp;
  b->desc = desc->desc;

  return true;
}

varbuf
keybindings::find(const char *action) {
  binding *b = bindings;
  while (b && strcmp(action, b->interp->action))
    b = b->next;
  if (!b)
    return varbuf(_("[not bound]"));

  const char *n= key2name(b->key);
  if (n)
    return varbuf(n);

  varbuf unknown;
  unknown.set_fmt(_("[unk: %d]"), b->key);
  return unknown;
}

const keybindings::interpretation *keybindings::operator()(int key) {
  binding *b = bindings;
  while (b && b->key != key)
    b = b->next;
  if (!b)
    return nullptr;
  return b->interp;
}

const char **keybindings::describenext() {
  binding *search;
  int count;
  for (;;) {
    if (!iterate->action)
      return nullptr;
    for (count=0, search=bindings; search; search=search->next)
      if (strcmp(search->interp->action, iterate->action) == 0)
        count++;
    if (count) break;
    iterate++;
  }
  const char **retarray= new const char *[count+2];
  retarray[0]= iterate->desc;
  for (count=1, search=bindings; search; search=search->next)
    if (strcmp(search->interp->action, iterate->action) == 0)
      retarray[count++]= key2name(search->key);
  retarray[count] = nullptr;
  iterate++;
  return retarray;
}

const char *keybindings::key2name(int key) {
  const keyname *search = keynames;
  while (search->key != -1 && search->key != key)
    search++;
  return search->kname;
}

int keybindings::name2key(const char *name) {
  const keyname *search = keynames;
  while (search->kname && strcasecmp(search->kname, name))
    search++;
  return search->key;
}

keybindings::~keybindings() {
  binding *search, *next;
  for (search=bindings; search; search=next) {
    next= search->next;
    delete search;
  }
}

const keybindings::description keybindings::descriptions[]= {
  // Actions which apply to both types of list.
  { "iscrollon",       N_("Scroll onwards through help/information")             },
  { "iscrollback",     N_("Scroll backwards through help/information")           },
  { "up",              N_("Move up")                                             },
  { "down",            N_("Move down")                                           },
  { "top",             N_("Go to top of list")                                   },
  { "bottom",          N_("Go to end of list")                                   },
  { "help",            N_("Request help (cycle through help screens)")           },
  { "info",            N_("Cycle through information displays")                  },
  { "redraw",          N_("Redraw display")                                      },
  { "scrollon1",       N_("Scroll onwards through list by 1 line")               },
  { "scrollback1",     N_("Scroll backwards through list by 1 line")             },
  { "iscrollon1",      N_("Scroll onwards through help/information by 1 line")   },
  { "iscrollback1",    N_("Scroll backwards through help/information by 1 line") },
  { "scrollon",        N_("Scroll onwards through list")                         },
  { "scrollback",      N_("Scroll backwards through list")                       },

  // Actions which apply only to lists of packages.
  { "install",         N_("Mark package(s) for installation")                    },
  { "remove",          N_("Mark package(s) for deinstallation")                  },
  { "purge",           N_("Mark package(s) for deinstall and purge")             },
  { "morespecific",    N_("Make highlight more specific")                        },
  { "lessspecific",    N_("Make highlight less specific")                        },
  { "search",          N_("Search for a package whose name contains a string")   },
  { "searchagain",     N_("Repeat last search")                                 },
  { "swaporder",       N_("Swap sort order priority/section")                    },
  { "quitcheck",       N_("Quit, confirming, and checking dependencies")         },
  { "quitnocheck",     N_("Quit, confirming without check")                      },
  { "quitrejectsug",   N_("Quit, rejecting conflict/dependency suggestions")     },
  { "abortnocheck",    N_("Abort - quit without making changes")                 },
  { "revert",          N_("Revert to old state for all packages")                },
  { "revertsuggest",   N_("Revert to suggested state for all packages")          },
  { "revertdirect",    N_("Revert to directly requested state for all packages") },
  { "revertinstalled", N_("Revert to currently installed state for all packages") },

  // Actions which apply only to lists of methods.
  { "select-and-quit", N_("Select currently-highlighted access method")          },
  { "abort",           N_("Quit without changing selected access method")        },
  { nullptr,           nullptr                                                   }
};
