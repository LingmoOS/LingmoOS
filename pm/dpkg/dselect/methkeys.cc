/*
 * dselect - Debian package maintenance user interface
 * methkeys.cc - method list keybindings
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

#include <dpkg/dpkg-db.h>

#include "dselect.h"
#include "bindings.h"

const keybindings::interpretation methodlist_kinterps[] = {
  { "up",              &methodlist::kd_up,           nullptr, qa_noquit           },
  { "down",            &methodlist::kd_down,         nullptr, qa_noquit           },
  { "top",             &methodlist::kd_top,          nullptr, qa_noquit           },
  { "bottom",          &methodlist::kd_bottom,       nullptr, qa_noquit           },
  { "scrollon",        &methodlist::kd_scrollon,     nullptr, qa_noquit           },
  { "scrollback",      &methodlist::kd_scrollback,   nullptr, qa_noquit           },
  { "iscrollon",       &methodlist::kd_iscrollon,    nullptr, qa_noquit           },
  { "iscrollback",     &methodlist::kd_iscrollback,  nullptr, qa_noquit           },
  { "scrollon1",       &methodlist::kd_scrollon1,    nullptr, qa_noquit           },
  { "scrollback1",     &methodlist::kd_scrollback1,  nullptr, qa_noquit           },
  { "iscrollon1",      &methodlist::kd_iscrollon1,   nullptr, qa_noquit           },
  { "iscrollback1",    &methodlist::kd_iscrollback1, nullptr, qa_noquit           },
  { "panon",           &methodlist::kd_panon,        nullptr, qa_noquit           },
  { "panback",         &methodlist::kd_panback,      nullptr, qa_noquit           },
  { "panon1",          &methodlist::kd_panon1,       nullptr, qa_noquit           },
  { "panback1",        &methodlist::kd_panback1,     nullptr, qa_noquit           },
  { "help",            &methodlist::kd_help,         nullptr, qa_noquit           },
  { "search",          &methodlist::kd_search,       nullptr, qa_noquit           },
  { "searchagain",     &methodlist::kd_searchagain,  nullptr, qa_noquit           },
  { "redraw",          &methodlist::kd_redraw,       nullptr, qa_noquit           },
  { "select-and-quit", &methodlist::kd_quit,         nullptr, qa_quitchecksave    },
  { "abort",           &methodlist::kd_abort,        nullptr, qa_quitnochecksave  },
  { nullptr,           nullptr,                      nullptr, qa_noquit           }
};

const keybindings::orgbinding methodlist_korgbindings[]= {
  { 'j',            "down"           }, // vi style
//{ 'n',            "down"           }, // no style
  { KEY_DOWN,       "down"           },
  { 'k',            "up"             }, // vi style
//{ 'p',            "up"             }, // no style
  { KEY_UP,         "up"             },

  { CTRL('f'),      "scrollon"       }, // vi style
  { 'N',            "scrollon"       },
  { KEY_NPAGE,      "scrollon"       },
  { ' ',            "scrollon"       },
  { CTRL('b'),      "scrollback"     }, // vi style
  { 'P',            "scrollback"     },
  { KEY_PPAGE,      "scrollback"     },
  { KEY_BACKSPACE,  "scrollback"     },
  { 0177,/*DEL*/    "scrollback"     },
  { CTRL('h'),      "scrollback"     },
  { CTRL('n'),      "scrollon1"      },
  { CTRL('p'),      "scrollback1"    },

  { 't',            "top"            },
  { KEY_HOME,       "top"            },
  { 'e',            "bottom"         },
  { KEY_LL,         "bottom"         },
  { KEY_END,        "bottom"         },

  { 'u',            "iscrollback"    },
  { 'd',            "iscrollon"      },
  { CTRL('u'),      "iscrollback1"   },
  { CTRL('d'),      "iscrollon1"     },

  { 'B',            "panback"        },
  { KEY_LEFT,       "panback"        },
  { 'F',            "panon"          },
  { KEY_RIGHT,      "panon"          },
  { CTRL('b'),      "panback1"       },
  { CTRL('f'),      "panon1"         },

  { '?',            "help"             },
  { KEY_HELP,       "help"             },
  { KEY_F(1),       "help"             },
  { '/',            "search"           },
  { 'n',            "searchagain"      },
  { '\\',           "searchagain"      },
  { CTRL('l'),      "redraw"           },

  { KEY_ENTER,      "select-and-quit"  },
  { '\r',           "select-and-quit"  },
  { 'x',            "abort"            },
  { 'X',            "abort"            },
  { 'Q',            "abort"            },

  {  -1,            nullptr            }
};
