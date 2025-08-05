/*
 * dselect - Debian package maintenance user interface
 * main.cc - main program
 *
 * Copyright © 1994-1996 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2000,2001 Wichert Akkerman <wakkerma@debian.org>
 * Copyright © 2006-2015 Guillem Jover <guillem@debian.org>
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
#include <sys/wait.h>

#include <errno.h>
#include <limits.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <ctype.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

// Solaris requires curses.h to be included before term.h
#include "dselect-curses.h"

#if defined(HAVE_NCURSESW_TERM_H)
#include <ncursesw/term.h>
#elif defined(HAVE_NCURSES_TERM_H)
#include <ncurses/term.h>
#else
#include <term.h>
#endif

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/options.h>
#include <dpkg/fsys.h>

#include "dselect.h"
#include "bindings.h"
#include "pkglist.h"

static const char printforhelp[] = N_("Type dselect --help for help.");

bool expertmode = false;

static keybindings packagelistbindings(packagelist_kinterps,packagelist_korgbindings);

struct table_t {
  const char *name;
  const int num;
};

static const struct table_t colortable[] = {
  {"black",	COLOR_BLACK	},
  {"red",	COLOR_RED	},
  {"green",	COLOR_GREEN	},
  {"yellow",	COLOR_YELLOW	},
  {"blue",	COLOR_BLUE	},
  {"magenta",	COLOR_MAGENTA	},
  {"cyan",	COLOR_CYAN	},
  {"white",	COLOR_WHITE	},
  {nullptr, 0},
};

static const struct table_t attrtable[]= {
  {"normal",	A_NORMAL	},
  {"standout",	A_STANDOUT	},
  {"underline",	A_UNDERLINE	},
  {"reverse",	A_REVERSE	},
  {"blink",	A_BLINK		},
  {"bright",	A_BLINK		}, // on some terminals
  {"dim",	A_DIM		},
  {"bold",	A_BOLD		},
  {nullptr, 0},
};

/* A slightly confusing mapping from dselect's internal names to
 * the user-visible names.*/
static const struct table_t screenparttable[]= {
  {"list",		list		},
  {"listsel",		listsel		},
  {"title",		title		},
  {"infohead",		thisstate	},
  {"pkgstate",		selstate	},
  {"pkgstatesel",	selstatesel	},
  {"listhead",		colheads	},
  {"query",		query		},
  {"info",		info_body	},
  {"infodesc",		info_head	},
  {"infofoot",		whatinfo	},
  {"helpscreen",	helpscreen	},
  {nullptr, 0},
};

/* Original colors. */
struct colordata color[]= {
  /* fore      back            attr */
  {COLOR_WHITE,        COLOR_BLACK,    0			}, // default, not used
  {COLOR_WHITE,        COLOR_BLACK,    0			}, // list
  {COLOR_WHITE,        COLOR_BLACK,    A_REVERSE		}, // listsel
  {COLOR_WHITE,        COLOR_RED,      0			}, // title
  {COLOR_WHITE,        COLOR_BLUE,     0			}, // thisstate
  {COLOR_WHITE,        COLOR_BLACK,    A_BOLD			}, // selstate
  {COLOR_WHITE,        COLOR_BLACK,    A_REVERSE | A_BOLD	}, // selstatesel
  {COLOR_WHITE,        COLOR_BLUE,     0			}, // colheads
  {COLOR_WHITE,        COLOR_RED,      0			}, // query
  {COLOR_WHITE,        COLOR_BLACK,    0			}, // info_body
  {COLOR_WHITE,        COLOR_BLACK,    A_BOLD			}, // info_head
  {COLOR_WHITE,        COLOR_BLUE,     0			}, // whatinfo
  {COLOR_WHITE,        COLOR_BLACK,    0			}, // help
};

struct menuentry {
  const char *command;
  const char *key;
  const char *option;
  const char *menuent;
  urqfunction *fn;
};

static const menuentry menuentries[]= {
  { "access",	N_("a"),	N_("[A]ccess"),	N_("Choose the access method to use."),			&urq_setup   },
  { "update",	N_("u"),	N_("[U]pdate"),	N_("Update list of available packages, if possible."),	&urq_update  },
  { "select",	N_("s"),	N_("[S]elect"),	N_("Request which packages you want on your system."),	&urq_list    },
  { "install",	N_("i"),	N_("[I]nstall"),N_("Install and upgrade wanted packages."),		&urq_install },
  { "config",	N_("c"),	N_("[C]onfig"),	N_("Configure any packages that are unconfigured."),	&urq_config  },
  { "remove",	N_("r"),	N_("[R]emove"),	N_("Remove unwanted software."),			&urq_remove  },
  { "quit",	N_("q"),	N_("[Q]uit"),	N_("Quit dselect."),					&urq_quit    },
  { nullptr,	nullptr,	N_("menu"),	nullptr,						&urq_menu    },
  { nullptr }
};

static const char programdesc[]=
      N_("Debian '%s' package handling frontend version %s.\n");

static const char licensestring[]= N_(
      "This is free software; see the GNU General Public License version 2 or\n"
      "later for copying conditions. There is NO warranty.\n");

static void DPKG_ATTR_NORET
printversion(const struct cmdinfo *ci, const char *value)
{
  printf(gettext(programdesc), DSELECT, PACKAGE_RELEASE);
  printf("%s", gettext(licensestring));

  m_output(stdout, _("<standard output>"));

  exit(0);
}

static void DPKG_ATTR_NORET
usage(const struct cmdinfo *ci, const char *value)
{
  int i;

  printf(_(
"Usage: %s [<option>...] [<command>...]\n"
"\n"), DSELECT);

  printf(_("Commands:\n"));
  for (i = 0; menuentries[i].command; i++)
    printf("  %-10s  %s\n", menuentries[i].command, menuentries[i].menuent);
  fputs("\n", stdout);

  printf(_(
"Options:\n"
"      --admindir <directory>       Use <directory> instead of %s.\n"
"      --instdir <directory>        Use <directory> instead of %s.\n"
"      --root <directory>           Use <directory> instead of %s.\n"
"      --expert                     Turn on expert mode.\n"
"  -D, --debug <file>               Turn on debugging, send output to <file>.\n"
"      --color <color-spec>         Configure screen colors.\n"
"      --colour <color-spec>        Ditto.\n"
), ADMINDIR, "/", "/");

  printf(_(
"  -?, --help                       Show this help message.\n"
"      --version                    Show the version.\n"
"\n"));

  printf(_("<color-spec> is <screen-part>:[<foreground>],[<background>][:<attr>[+<attr>]...]\n"));

  printf(_("<screen-part> is:"));
  for (i=0; screenparttable[i].name; i++)
    printf(" %s", screenparttable[i].name);
  fputs("\n", stdout);

  printf(_("<color> is:"));
  for (i = 0; colortable[i].name; i++)
    printf(" %s", colortable[i].name);
  fputs("\n", stdout);

  printf(_("<attr> is:"));
  for (i=0; attrtable[i].name; i++)
    printf(" %s", attrtable[i].name);
  fputs("\n", stdout);

  m_output(stdout, _("<standard output>"));

  exit(0);
}

/* These are called by C code, so need to have C calling convention */
extern "C" {

  static void
  set_debug(const struct cmdinfo*, const char *v)
  {
    FILE *fp;

    fp = fopen(v, "a");
    if (!fp)
      ohshite(_("couldn't open debug file '%.255s'\n"), v);

    debug_set_output(fp, v);
    debug_set_mask(dbg_general | dbg_depcon);
  }

  static void
  set_expert(const struct cmdinfo*, const char *v)
  {
    expertmode = true;
  }

  static int
  findintable(const struct table_t *table, const char *item, const char *tablename)
  {
    int i;

    for (i = 0; item && (table[i].name != nullptr); i++)
      if (strcasecmp(item, table[i].name) == 0)
        return table[i].num;

    ohshit(_("invalid %s '%s'"), tablename, item);
  }

  /*
   *  The string's format is:
   *    screenpart:[forecolor][,backcolor][:[<attr>, ...]
   *  Examples: --color title:black,cyan:bright+underline
   *            --color list:red,yellow
   *            --color colheads:,green:bright
   *            --color selstate::reverse  // doesn't work FIXME
   */
  static void
  set_color(const struct cmdinfo*, const char *string)
  {
    char *s;
    char *colors, *attributes;
    int screenpart;

    s = m_strdup(string); // strtok modifies strings, keep string const
    screenpart= findintable(screenparttable, strtok(s, ":"), _("screen part"));
    colors = strtok(nullptr, ":");
    attributes = strtok(nullptr, ":");

    if ((colors == nullptr || ! strlen(colors)) &&
        (attributes == nullptr || ! strlen(attributes))) {
       ohshit(_("missing color specification"));
    }

    if (colors != nullptr && strlen(colors)) {
      char *colorname;

      colorname = strtok(colors, ",");
      if (colorname != nullptr && strlen(colorname)) {
        // normalize attributes to prevent confusion
        color[screenpart].attr= A_NORMAL;
        color[screenpart].fore = findintable(colortable, colorname, _("color"));
      }
      colorname = strtok(nullptr, ",");
      if (colorname != nullptr && strlen(colorname)) {
        color[screenpart].attr= A_NORMAL;
        color[screenpart].back = findintable(colortable, colorname, _("color"));
      }
    }

    if (attributes != nullptr && strlen(attributes)) {
      for (char *attrib = strtok(attributes, "+");
           attrib != nullptr && strlen(attrib);
           attrib = strtok(nullptr, "+")) {
               int aval;

               aval = findintable(attrtable, attrib, _("color attribute"));
               if (aval == A_NORMAL) // set to normal
                       color[screenpart].attr= aval;
               else // add to existing attribs
                       color[screenpart].attr= color[screenpart].attr | aval;
      }
    }

    free(s);
  }

} /* End of extern "C" */

static const struct cmdinfo cmdinfos[]= {
  { "admindir",     0,  1, nullptr,  nullptr,   set_admindir, 1 },
  { "instdir",      0,  1, nullptr,  nullptr,   set_instdir, 1 },
  { "root",         0,  1, nullptr,  nullptr,   set_root, 1  },
  { "debug",       'D', 1, nullptr,  nullptr,   set_debug    },
  { "expert",      'E', 0, nullptr,  nullptr,   set_expert   },
  { "help",        '?', 0, nullptr,  nullptr,   usage        },
  { "version",      0,  0, nullptr,  nullptr,   printversion },
  { "color",        0,  1, nullptr,  nullptr,   set_color    }, /* US spelling */
  { "colour",       0,  1, nullptr,  nullptr,   set_color    }, /* UK spelling */
  { nullptr,        0,  0, nullptr,  nullptr,   nullptr      }
};

static bool cursesareon = false;
void curseson() {
  if (!cursesareon) {
    const char *cup, *smso;
    initscr();
    cup= tigetstr("cup");
    smso= tigetstr("smso");
    if (!cup || !smso) {
      endwin();
      if (!cup)
        fputs(_("Terminal does not appear to support cursor addressing.\n"),stderr);
      if (!smso)
        fputs(_("Terminal does not appear to support highlighting.\n"),stderr);
      fprintf(stderr,
	      _("Set your TERM variable correctly, use a better terminal,\n"
	        "or make do with the per-package management tool %s.\n"),
	      DPKG);
      ohshit(_("terminal lacks necessary features, giving up"));
    }
  }
  cursesareon = true;
}

void cursesoff() {
  if (cursesareon) {
    clear();
    refresh();
    endwin();
  }
  cursesareon = false;
}

urqresult urq_list(void) {
  modstatdb_open(static_cast<modstatdb_rw>(msdbrw_writeifposs |
                                           msdbrw_available_readonly));

  curseson();

  packagelist *l= new packagelist(&packagelistbindings);
  l->resolvesuggest();
  l->display();
  delete l;

  modstatdb_shutdown();

  return urqr_normal;
}

static void
display_menu_entry(int i, int so)
{
  const menuentry *me= &menuentries[i];

  varbuf buf;
  buf.add_fmt(" %c %d. %-11.11s %-80.80s ",
              so ? '*' : ' ', i,
              gettext(me->option),
              gettext(me->menuent));

  int x, y DPKG_ATTR_UNUSED;
  getmaxyx(stdscr,y,x);

  attrset(so ? A_REVERSE : A_NORMAL);
  mvaddnstr(i + 2, 0, buf.str(), x - 1);
  attrset(A_NORMAL);
}

static int
refreshmenu(void)
{
  curseson(); cbreak(); noecho(); nonl(); keypad(stdscr,TRUE);

  int x, y DPKG_ATTR_UNUSED;
  getmaxyx(stdscr,y,x);

  varbuf buf;
  buf.add_fmt(gettext(programdesc), DSELECT, PACKAGE_RELEASE);

  clear();
  attrset(A_BOLD);
  mvaddnstr(0, 0, buf.str(), x - 1);

  attrset(A_NORMAL);
  const struct menuentry *mep; int i;
  for (mep=menuentries, i=0; mep->option && mep->menuent; mep++, i++)
    display_menu_entry(i, 0);

  attrset(A_BOLD);
  addstr(_("\n\n"
         "Move around with ^P and ^N, cursor keys, initial letters, or digits;\n"
         "Press <enter> to confirm selection.   ^L redraws screen.\n\n"));

  attrset(A_NORMAL);
  addstr(_("Copyright (C) 1994-1996 Ian Jackson.\n"
           "Copyright (C) 2000,2001 Wichert Akkerman.\n"));
  addstr(gettext(licensestring));

  modstatdb_init();
  if (!modstatdb_can_lock())
    addstr(_("\n\n"
             "Read-only access: only preview of selections is available!"));
  modstatdb_done();

  return i;
}

urqresult urq_menu(void) {
  int entries, c;
  entries= refreshmenu();
  int cursor=0;
  display_menu_entry(0, 1);
  for (;;) {
    refresh();
    do {
      c= getch();
      if (c == KEY_RESIZE) {
        refreshmenu();
        display_menu_entry(cursor, 1);
        continue;
      }
    } while (c == ERR && errno == EINTR);
    if (c==ERR)  {
      if(errno != 0)
        ohshite(_("failed to getch in main menu"));
      else {
        clearok(stdscr, TRUE);
        clear();
        refreshmenu();
        display_menu_entry(cursor, 1);
      }
    }

    if (c == CTRL('n') || c == KEY_DOWN || c == ' ' || c == 'j') {
      display_menu_entry(cursor, 0);
      cursor++;
      cursor %= entries;
      display_menu_entry(cursor, 1);
    } else if (c == CTRL('p') || c == KEY_UP || c == CTRL('h') ||
               c==KEY_BACKSPACE || c==KEY_DC || c=='k') {
      display_menu_entry(cursor, 0);
      cursor += entries - 1;
      cursor %= entries;
      display_menu_entry(cursor, 1);
    } else if (c=='\n' || c=='\r' || c==KEY_ENTER) {
      clear(); refresh();

      /* FIXME: trap errors in urq_... */
      urqresult res = menuentries[cursor].fn();
      switch (res) {
      case urqr_quitmenu:
        return urqr_quitmenu;
      case urqr_normal:
        cursor++; cursor %= entries;
      case urqr_fail:
        break;
      default:
        internerr("unknown menufn %d", res);
      }
      refreshmenu();
      display_menu_entry(cursor, 1);
    } else if (c == CTRL('l')) {
      clearok(stdscr, TRUE);
      clear();
      refreshmenu();
      display_menu_entry(cursor, 1);
    } else if (isdigit(c)) {
      char buf[2]; buf[0]=c; buf[1]=0; c=atoi(buf);
      if (c < entries) {
        display_menu_entry(cursor, 0);
        cursor = c;
        display_menu_entry(cursor, 1);
      } else {
        beep();
      }
    } else if (isalpha(c)) {
      c= tolower(c);
      int i = 0;
      while (i < entries && gettext(menuentries[i].key)[0] != c)
        i++;
      if (i < entries) {
        display_menu_entry(cursor, 0);
        cursor = i;
        display_menu_entry(cursor, 1);
      } else {
        beep();
      }
    } else {
      beep();
    }
  }
}

urqresult urq_quit(void) {
  /* FIXME: check packages OK. */
  return urqr_quitmenu;
}

static void
dselect_catch_fatal_error()
{
  cursesoff();
  catch_fatal_error();
}

int
main(int, const char *const *argv)
{
  dpkg_locales_init(DSELECT);
  dpkg_set_progname(DSELECT);

  push_error_context_func(dselect_catch_fatal_error, print_fatal_error, nullptr);

  dpkg_options_load(DSELECT, cmdinfos);
  dpkg_options_parse(&argv, cmdinfos, printforhelp);

  debug(dbg_general, "root=%s admindir=%s", dpkg_fsys_get_dir(), dpkg_db_get_dir());

  if (*argv) {
    const char *a;
    while ((a = *argv++) != nullptr) {
      const menuentry *me = menuentries;
      while (me->command && strcmp(me->command, a))
        me++;
      if (!me->command)
        badusage(_("unknown action string '%.50s'"), a);
      me->fn();
    }
  } else {
    urq_menu();
  }

  cursesoff();
  dpkg_program_done();
  dpkg_locales_done();

  return(0);
}
