/*
 * dselect - Debian package maintenance user interface
 * pkgkeys.cc - package list keybindings
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

const keybindings::interpretation packagelist_kinterps[] = {
  { "up",               nullptr,  &packagelist::kd_up,               qa_noquit           },
  { "down",             nullptr,  &packagelist::kd_down,             qa_noquit           },
  { "top",              nullptr,  &packagelist::kd_top,              qa_noquit           },
  { "bottom",           nullptr,  &packagelist::kd_bottom,           qa_noquit           },
  { "scrollon",         nullptr,  &packagelist::kd_scrollon,         qa_noquit           },
  { "scrollback",       nullptr,  &packagelist::kd_scrollback,       qa_noquit           },
  { "iscrollon",        nullptr,  &packagelist::kd_iscrollon,        qa_noquit           },
  { "iscrollback",      nullptr,  &packagelist::kd_iscrollback,      qa_noquit           },
  { "scrollon1",        nullptr,  &packagelist::kd_scrollon1,        qa_noquit           },
  { "scrollback1",      nullptr,  &packagelist::kd_scrollback1,      qa_noquit           },
  { "iscrollon1",       nullptr,  &packagelist::kd_iscrollon1,       qa_noquit           },
  { "iscrollback1",     nullptr,  &packagelist::kd_iscrollback1,     qa_noquit           },
  { "panon",            nullptr,  &packagelist::kd_panon,            qa_noquit           },
  { "panback",          nullptr,  &packagelist::kd_panback,          qa_noquit           },
  { "panon1",           nullptr,  &packagelist::kd_panon1,           qa_noquit           },
  { "panback1",         nullptr,  &packagelist::kd_panback1,         qa_noquit           },
  { "install",          nullptr,  &packagelist::kd_select,           qa_noquit           },
  { "remove",           nullptr,  &packagelist::kd_deselect,         qa_noquit           },
  { "purge",            nullptr,  &packagelist::kd_purge,            qa_noquit           },
  { "hold",             nullptr,  &packagelist::kd_hold,             qa_noquit           },
  { "unhold",           nullptr,  &packagelist::kd_unhold,           qa_noquit           },
  { "info",             nullptr,  &packagelist::kd_info,             qa_noquit           },
  { "toggleinfo",       nullptr,  &packagelist::kd_toggleinfo,       qa_noquit           },
  { "verbose",          nullptr,  &packagelist::kd_verbose,          qa_noquit           },
  { "archdisplay",      nullptr,  &packagelist::kd_archdisplay,      qa_noquit           },
  { "versiondisplay",   nullptr,  &packagelist::kd_versiondisplay,   qa_noquit           },
  { "help",             nullptr,  &packagelist::kd_help,             qa_noquit           },
  { "search",           nullptr,  &packagelist::kd_search,           qa_noquit           },
  { "searchagain",      nullptr,  &packagelist::kd_searchagain,      qa_noquit           },
  { "swaporder",        nullptr,  &packagelist::kd_swaporder,        qa_noquit           },
  { "swapstatorder",    nullptr,  &packagelist::kd_swapstatorder,    qa_noquit           },
  { "redraw",           nullptr,  &packagelist::kd_redraw,           qa_noquit           },
  { "quitcheck",        nullptr,  &packagelist::kd_quit_noop,        qa_quitchecksave    },
  { "quitrejectsug",    nullptr,  &packagelist::kd_revertdirect,     qa_quitnochecksave  },
  { "quitnocheck",      nullptr,  &packagelist::kd_quit_noop,        qa_quitnochecksave  },
  { "abortnocheck",     nullptr,  &packagelist::kd_revert_abort,     qa_quitnochecksave  },
  { "revert",           nullptr,  &packagelist::kd_revert_abort,     qa_noquit           },
  { "revertsuggest",    nullptr,  &packagelist::kd_revertsuggest,    qa_noquit           },
  { "revertdirect",     nullptr,  &packagelist::kd_revertdirect,     qa_noquit           },
  { "revertinstalled",  nullptr,  &packagelist::kd_revertinstalled,  qa_noquit           },
  { nullptr,            nullptr,  nullptr,                           qa_noquit           }
};

const keybindings::orgbinding packagelist_korgbindings[]= {
  { 'j',            "down"             }, // vi style
  { KEY_DOWN,       "down"             },
  { 'k',            "up"               }, // vi style
  { 'p',            "up"               },
  { KEY_UP,         "up"               },

  { 'N',            "scrollon"         },
  { KEY_NPAGE,      "scrollon"         },
  { ' ',            "scrollon"         },
  { 'P',            "scrollback"       },
  { KEY_PPAGE,      "scrollback"       },
  { KEY_BACKSPACE,  "scrollback"       },
  { 0177,           "scrollback"       }, // ASCII DEL
  { CTRL('h'),      "scrollback"       },
  { CTRL('n'),      "scrollon1"        },
  { CTRL('p'),      "scrollback1"      },

  { 't',            "top"              },
  { KEY_HOME,       "top"              },
  { 'e',            "bottom"           },
  { KEY_LL,         "bottom"           },
  { KEY_END,        "bottom"           },

  { 'u',            "iscrollback"      },
  { 'd',            "iscrollon"        },
  { CTRL('u'),      "iscrollback1"     },
  { CTRL('d'),      "iscrollon1"       },

  { 'B',            "panback"          },
  { KEY_LEFT,       "panback"          },
  { 'F',            "panon"            },
  { KEY_RIGHT,      "panon"            },
  { CTRL('b'),      "panback1"         },
  { CTRL('f'),      "panon1"           },

  { '+',            "install"          },
  { KEY_IC,         "install"          },
  { '-',            "remove"           },
  { KEY_DC,         "remove"           },
  { '_',            "purge"            },
  { 'H',            "hold"             },
  { '=',            "hold"             },
  { 'G',            "unhold"           },

  { '?',            "help"             },
  { KEY_HELP,       "help"             },
  { KEY_F(1),       "help"             },
  { 'i',            "info"             },
  { 'I',            "toggleinfo"       },
  { 'o',            "swaporder"        },
  { 'O',            "swapstatorder"    },
  { 'v',            "verbose"          },
  { 'A',            "archdisplay"      },
  { 'V',            "versiondisplay"   },
  { CTRL('l'),      "redraw"           },
  { '/',            "search"           },
  { 'n',            "searchagain"      },
  { '\\',           "searchagain"      },

  { KEY_ENTER,      "quitcheck"        },
  { '\r',           "quitcheck"        },
  { 'Q',            "quitnocheck"      },
  { 'x',            "abortnocheck"     },
  { 'X',            "abortnocheck"     },
  { 'R',            "revert"           },
  { 'U',            "revertsuggest"    },
  { 'D',            "revertdirect"     },
  { 'C',            "revertinstalled"  },

  {  -1,            nullptr            }
};
