/*
 * dselect - Debian package maintenance user interface
 * baselist.cc - list of somethings
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2001 Wichert Akkerman <wakkerma@debian.org>
 * Copyright © 2007-2013 Guillem Jover <guillem@debian.org>
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

#include <sys/ioctl.h>

#include <errno.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>

#include "dselect.h"
#include "bindings.h"

void mywerase(WINDOW *win) {
  int my,mx,y,x;
  getmaxyx(win,my,mx);
  for (y=0; y<my; y++) {
    wmove(win,y,0); for (x=0; x<mx; x++) waddch(win,' ');
  }
  wmove(win,0,0);
}

void
baselist::resize_window()
{
  debug(dbg_general, "baselist::resize_window(), baselist=%p", this);
  enddisplay();
  startdisplay();
  if (doupdate() == ERR)
    ohshite(_("cannot update screen after window resize"));
}

void
baselist::add_column(column &col, const char *title, int width)
{
  col.title = title;
  col.x = col_cur_x;
  col.width = width;

  col_cur_x += col.width + gap_width;
}

void
baselist::end_column(column &col, const char *title)
{
  col.title = title;
  col.x = col_cur_x;
  col.width = total_width - col.x;

  col_cur_x += col.width + gap_width;
}

void
baselist::draw_column_head(const column &col)
{
  mvwaddnstr(colheadspad, 0, col.x, col.title, col.width);
}

void
baselist::draw_column_sep(const column &col, int y)
{
  mvwaddch(listpad, y, col.x - 1, ' ');
}

void
baselist::draw_column_item(const column &col, int y, const char *item)
{
  mvwprintw(listpad, y, col.x, "%-*.*s", col.width, col.width, item);
}

void baselist::setheights() {
  int y= ymax - (title_height + colheads_height + thisstate_height);

  if (y < 1)
    internerr("widget y=%d < 1", y);

  if (showinfo==2 && y>=7) {
    list_height= 5;
    whatinfo_height= 1;
    info_height= y-6;
  } else if (showinfo==1 && y>=10) {
    list_height= y/2;
    info_height= (y-1)/2;
    whatinfo_height= 1;
  } else {
    list_height= y;
    info_height= 0;
    whatinfo_height= 0;
  }
  colheads_row= title_height;
  list_row= colheads_row + colheads_height;
  thisstate_row= list_row + list_height;
  info_row= thisstate_row + thisstate_height;
  whatinfo_row= ymax - 1;
}

void baselist::startdisplay() {
  debug(dbg_general, "baselist[%p]::startdisplay()", this);
  cbreak(); noecho(); nonl(); keypad(stdscr,TRUE);
  clear(); wnoutrefresh(stdscr);

  // find attributes
  if (has_colors() && start_color()==OK && COLOR_PAIRS >= numscreenparts) {
    int i;
    printf("allocing\n");
    for (i = 1; i < numscreenparts; i++) {
       if (init_pair(i, color[i].fore, color[i].back) != OK)
         ohshite(_("cannot allocate color pair"));
       part_attr[i] = COLOR_PAIR(i) | color[i].attr;
    }
  } else {
    /* User defined attributes for B&W mode are not currently supported. */
    part_attr[title] = A_REVERSE;
    part_attr[thisstate] = A_STANDOUT;
    part_attr[list] = 0;
    part_attr[listsel] = A_STANDOUT;
    part_attr[selstate] = A_BOLD;
    part_attr[selstatesel] = A_STANDOUT;
    part_attr[colheads]= A_BOLD;
    part_attr[query] = part_attr[title];
    part_attr[info_body] = part_attr[list];
    part_attr[info_head] = A_BOLD;
    part_attr[whatinfo] = part_attr[thisstate];
    part_attr[helpscreen] = A_NORMAL;
  }

  // set up windows and pads, based on screen size
  getmaxyx(stdscr,ymax,xmax);
  title_height= ymax>=6;
  colheads_height= ymax>=5;
  thisstate_height= ymax>=3;

  setheights();
  setwidths();

  titlewin= newwin(1,xmax, 0,0);
  if (!titlewin) ohshite(_("failed to create title window"));
  wattrset(titlewin, part_attr[title]);

  whatinfowin= newwin(1,xmax, whatinfo_row,0);
  if (!whatinfowin) ohshite(_("failed to create whatinfo window"));
  wattrset(whatinfowin, part_attr[whatinfo]);

  listpad = newpad(ymax, total_width);
  if (!listpad) ohshite(_("failed to create baselist pad"));

  colheadspad= newpad(1, total_width);
  if (!colheadspad) ohshite(_("failed to create heading pad"));
  wattrset(colheadspad, part_attr[colheads]);

  thisstatepad= newpad(1, total_width);
  if (!thisstatepad) ohshite(_("failed to create thisstate pad"));
  wattrset(thisstatepad, part_attr[thisstate]);

  infopad= newpad(MAX_DISPLAY_INFO, total_width);
  if (!infopad) ohshite(_("failed to create info pad"));
  wattrset(infopad, part_attr[info_body]);
  wbkgdset(infopad, ' ' | part_attr[info_body]);

  querywin= newwin(1,xmax,ymax-1,0);
  if (!querywin) ohshite(_("failed to create query window"));
  wbkgdset(querywin, ' ' | part_attr[query]);

  if (cursorline >= topofscreen + list_height) topofscreen= cursorline;
  if (topofscreen > nitems - list_height) topofscreen= nitems - list_height;
  if (topofscreen < 0) topofscreen= 0;

  infotopofscreen= 0; leftofscreen= 0;

  redrawall();

  debug(dbg_general,
        "baselist::startdisplay() done ...\n\n"
        " xmax=%d, ymax=%d;\n\n"
        " title_height=%d, colheads_height=%d, list_height=%d;\n"
        " thisstate_height=%d, info_height=%d, whatinfo_height=%d;\n\n"
        " colheads_row=%d, thisstate_row=%d, info_row=%d;\n"
        " whatinfo_row=%d, list_row=%d;\n\n",
        xmax, ymax, title_height, colheads_height, list_height,
        thisstate_height, info_height, whatinfo_height,
        colheads_row, thisstate_row, info_row, whatinfo_row, list_row);
}

void baselist::enddisplay() {
  delwin(titlewin);
  delwin(whatinfowin);
  delwin(listpad);
  delwin(colheadspad);
  delwin(thisstatepad);
  delwin(infopad);
  wmove(stdscr,ymax,0); wclrtoeol(stdscr);
  listpad = nullptr;
  col_cur_x = 0;
}

void baselist::redrawall() {
  redrawtitle();
  redrawcolheads();
  wattrset(listpad, part_attr[list]);
  mywerase(listpad);
  ldrawnstart= ldrawnend= -1; // start is first drawn; end is first undrawn; -1=none
  refreshlist();
  redrawthisstate();
  redrawinfo();
}

void baselist::redraw1item(int index) {
  redraw1itemsel(index, index == cursorline);
}

baselist::baselist(keybindings *kb) {
  debug(dbg_general, "baselist[%p]::baselist()", this);

  bindings= kb;
  nitems= 0;

  col_cur_x = 0;
  gap_width = 1;
  total_width = max(TOTAL_LIST_WIDTH, COLS);

  xmax= -1;
  ymax = -1;

  list_height = 0;
  info_height = 0;
  title_height = 0;
  whatinfo_height = 0;
  colheads_height = 0;
  thisstate_height = 0;

  list_row = 0;
  info_row = 0;
  whatinfo_row = 0;
  colheads_row = 0;
  thisstate_row = 0;

  topofscreen = 0;
  leftofscreen = 0;
  infotopofscreen = 0;
  infolines = 0;

  listpad = nullptr;
  infopad = nullptr;
  colheadspad = nullptr;
  thisstatepad = nullptr;
  titlewin = nullptr;
  querywin = nullptr;
  whatinfowin = nullptr;

  cursorline = -1;
  ldrawnstart = 0;
  ldrawnend = 0;
  showinfo= 1;

  searchstring.reset();
}

void baselist::itd_keys() {
  whatinfovb += _("Keybindings");

  const int givek= xmax/3;
  bindings->describestart();
  const char **ta;
  while ((ta = bindings->describenext()) != nullptr) {
    const char **tap= ta+1;
    for (;;) {
      waddstr(infopad, gettext(*tap));
      tap++;  if (!*tap) break;
      waddstr(infopad, ", ");
    }
    int y,x;
    getyx(infopad,y,x);
    if (x >= givek) y++;
    mvwaddstr(infopad, y,givek, ta[0]);
    waddch(infopad,'\n');
    delete [] ta;
  }
}

void baselist::dosearch() {
  int offset, index;
  debug(dbg_general, "baselist[%p]::dosearch(); searchstring='%s'",
        this, searchstring.str());
  for (offset = 1, index = max(topofscreen, cursorline + 1);
       offset<nitems;
       offset++, index++) {
    if (index >= nitems) index -= nitems;
    if (matchsearch(index)) {
      topofscreen= index-1;
      if (topofscreen > nitems - list_height) topofscreen= nitems-list_height;
      if (topofscreen < 0) topofscreen= 0;
      setcursor(index);
      return;
    }
  }
  beep();
}

void baselist::refreshinfo() {
  pnoutrefresh(infopad, infotopofscreen,leftofscreen, info_row,0,
               min(info_row + info_height - 1, info_row + MAX_DISPLAY_INFO - 1),
               min(total_width - leftofscreen - 1, xmax - 1));

  if (whatinfo_height) {
    mywerase(whatinfowin);
    mvwaddstr(whatinfowin,0,0, whatinfovb.str());
    if (infolines > info_height) {
      wprintw(whatinfowin,_("  -- %d%%, press "),
              (infotopofscreen + info_height) * 100 / infolines);
      if (infotopofscreen + info_height < infolines) {
        wprintw(whatinfowin, _("%s for more"), bindings->find("iscrollon").str());
        if (infotopofscreen) waddstr(whatinfowin, ", ");
      }
      if (infotopofscreen)
        wprintw(whatinfowin, _("%s to go back"), bindings->find("iscrollback").str());
      waddch(whatinfowin,'.');
    }
    wnoutrefresh(whatinfowin);
  }
}

void baselist::wordwrapinfo(int offset, const char *m) {
  ssize_t usemax = xmax - 5;
  debug(dbg_general, "baselist[%p]::wordwrapinfo(%d, '%s')", this, offset, m);
  bool wrapping = false;

  for (;;) {
    int offleft=offset; while (*m == ' ' && offleft>0) { m++; offleft--; }
    const char *p = strchrnul(m, '\n');
    ptrdiff_t l = p - m;

    while (l && c_isspace(m[l - 1]))
      l--;
    if (!l || (*m == '.' && l == 1)) {
      if (wrapping) waddch(infopad,'\n');
      waddch(infopad, '\n');
      wrapping = false;
    } else if (*m == ' ' || usemax < 10) {
      if (wrapping) waddch(infopad,'\n');
      waddnstr(infopad, m, l);
      waddch(infopad, '\n');
      wrapping = false;
    } else {
      int x, y DPKG_ATTR_UNUSED;

      if (wrapping) {
        getyx(infopad, y,x);
        if (x+1 >= usemax) {
          waddch(infopad,'\n');
        } else {
          waddch(infopad,' ');
        }
      }
      for (;;) {
        getyx(infopad, y,x);
        ssize_t dosend = usemax - x;
        if (l <= dosend) {
          dosend=l;
        } else {
          ssize_t i = dosend;
          while (i > 0 && m[i] != ' ') i--;
          if (i > 0 || x > 0) dosend=i;
        }
        if (dosend) waddnstr(infopad, m, dosend);
        while (dosend < l && m[dosend] == ' ') dosend++;
        l-= dosend; m+= dosend;
        if (l <= 0) break;
        waddch(infopad,'\n');
      }
      wrapping = true;
    }
    if (*p == '\0')
      break;
    if (getcury(infopad) == (MAX_DISPLAY_INFO - 1)) {
      waddstr(infopad,
              "[The package description is too long and has been truncated...]");
      break;
    }
    m= ++p;
  }
  debug(dbg_general, "baselist[%p]::wordwrapinfo() done", this);
}

baselist::~baselist() { }
