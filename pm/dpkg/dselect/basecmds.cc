/*
 * dselect - Debian package maintenance user interface
 * basecmds.cc - base list keyboard commands display
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2000,2001 Wichert Akkerman <wakkerma@debian.org>
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

#include <utility>

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>

#include "dselect.h"
#include "helpmsgs.h"

void baselist::kd_scrollon() {
  topofscreen += list_height;
  if (topofscreen > nitems - list_height) topofscreen= nitems - list_height;
  if (topofscreen < 0) topofscreen= 0;
  if (cursorline < topofscreen)
    setcursor(topofscreen);
  refreshlist();
}

void baselist::kd_scrollon1() {
  if (topofscreen >= nitems - list_height) return;
  topofscreen++;
  if (cursorline < topofscreen)
    setcursor(topofscreen);
  refreshlist();
}

void baselist::kd_scrollback() {
  topofscreen -= list_height;
  if (topofscreen < 0) topofscreen= 0;
  if (cursorline >= topofscreen + list_height)
    setcursor(topofscreen + list_height - 1);
  refreshlist();
}

void baselist::kd_scrollback1() {
  if (topofscreen <= 0) return;
  topofscreen--;
  if (cursorline >= topofscreen + list_height)
    setcursor(topofscreen + list_height - 1);
  refreshlist();
}

void baselist::kd_top() {
  topofscreen= 0; setcursor(0);
}

void baselist::kd_bottom() {
  topofscreen= nitems - list_height;
  if (topofscreen < 0) topofscreen= 0;
  setcursor(min(topofscreen + list_height - 1, nitems - 1));
}

void baselist::kd_redraw() {
  clearok(curscr,TRUE);
  redrawall();
  debug(dbg_general, "baselist[%p]::kd_redraw() done", this);
}

void baselist::kd_searchagain() {
  if (!searchstring.len()) {
    beep();
    return;
  }
  dosearch();
}

bool
baselist::checksearch(varbuf &str)
{
  return true;
}

bool
baselist::matchsearch(int index)
{
  int lendiff, i;
  const char *name;

  name = itemname(index);
  if (!name)
    return false;	/* Skip things without a name (separators). */

  lendiff = strlen(name) - searchstring.len();
  for (i=0; i<=lendiff; i++)
    if (strncasecmp(name + i, searchstring.str(), searchstring.len()) == 0)
      return true;

  return false;
}

void baselist::kd_search() {
  varbuf newsearchstring(128);
  newsearchstring = searchstring;

  werase(querywin);
  mvwaddstr(querywin,0,0, _("Search for ? "));
  echo();
  if (wgetnstr(querywin, newsearchstring.buf, newsearchstring.size - 1) == ERR)
    searchstring.reset();
  newsearchstring.trunc(strlen(newsearchstring.buf));
  resize_window();
  noecho();
  if (whatinfo_height) { touchwin(whatinfowin); refreshinfo(); }
  else if (info_height) { touchwin(infopad); refreshinfo(); }
  else if (thisstate_height) redrawthisstate();
  else { touchwin(listpad); refreshlist(); }
  if (newsearchstring.len()) {
    if (!checksearch(newsearchstring)) {
      beep();
      return;
    }

    searchstring = std::move(newsearchstring);
    dosearch();
  } else
    kd_searchagain();
}

void
baselist::displayerror(const char *str)
{
  const char *pr = _("Error: ");
  int prlen=strlen(pr);

  beep();
  werase(querywin);
  mvwaddstr(querywin,0,0,pr);
  mvwaddstr(querywin,0,prlen,str);
  wgetch(querywin);
  if (whatinfo_height) { touchwin(whatinfowin); refreshinfo(); }
  else if (info_height) { touchwin(infopad); refreshinfo(); }
  else if (thisstate_height) redrawthisstate();
}


void baselist::displayhelp(const struct helpmenuentry *helpmenu, int key) {
  int maxx, maxy, i, nextkey;

  wbkgdset(stdscr, ' ' | part_attr[helpscreen]);
  clearok(stdscr,TRUE);
  for (;;) {
    getmaxyx(stdscr, maxy, maxx);
    werase(stdscr);
    const struct helpmenuentry *hme = helpmenu;
    while (hme->key && hme->key != key)
      hme++;
    if (hme->key) {
      int x, y DPKG_ATTR_UNUSED;

      attrset(part_attr[helpscreen]);
      mvaddstr(1,0, gettext(hme->msg->text));
      attrset(part_attr[title]);
      mvaddstr(0,0, _("Help: "));
      addstr(gettext(hme->msg->title));
      getyx(stdscr,y,x);
      while (++x<maxx) addch(' ');
      attrset(part_attr[thisstate]);
      mvaddstr(maxy-1,0,
_("Press ? for help menu, . for next topic, <space> to exit help."));
      getyx(stdscr,y,x);
      while (++x<maxx) addch(' ');
      move(maxy,maxx);
      attrset(A_NORMAL);
      nextkey= hme[1].key;
    } else {
      mvaddstr(1,1, _("Help information is available under the following topics:"));
      for (i=0, hme=helpmenu; hme->key; hme++,i++) {
        attrset(A_BOLD);
        mvaddch(i+3,3, hme->key);
        attrset(A_NORMAL);
        mvaddstr(i+3,6, gettext(hme->msg->title));
      }
      mvaddstr(i+4,1,
               _("Press a key from the list above, <space> or 'q' to exit help,\n"
                 "  or '.' (full stop) to read each help page in turn. "));
      nextkey= helpmenu[0].key;
    }
    refresh();
    do {
      int curkey = key;

      key = getch();
      if (key == KEY_RESIZE) {
        resize_window();
        key = curkey;
      }
    } while (key == ERR && errno == EINTR);
    if (key == EOF) ohshite(_("error reading keyboard in help"));
    if (key == ' ' || key == 'q') {
      break;
    } else if (key == '?' || key == 'h') {
      key= 0;
    } else if (key == '.') {
      key= nextkey;
    }
  }
  werase(stdscr);
  clearok(stdscr,TRUE);
  wnoutrefresh(stdscr);

  redrawtitle();
  refreshcolheads();
  refreshlist();
  redrawthisstate();
  refreshinfo();
  wnoutrefresh(whatinfowin);
}

void baselist::kd_help() {
  displayhelp(helpmenulist(),0);
}

void baselist::setcursor(int index) {
  if (listpad && cursorline != -1) {
    redrawitemsrange(cursorline,cursorline+1);
    redraw1itemsel(cursorline,0);
  }
  if (cursorline != index) infotopofscreen= 0;
  cursorline= index;
  if (listpad) {
    redrawitemsrange(cursorline,cursorline+1);
    redraw1itemsel(cursorline,1);
    refreshlist();
    redrawthisstate();
  }
  redrawinfo();
}

void baselist::kd_down() {
  int ncursor= cursorline;
  // scroll by one line unless the bottom is already visible
  // or we're in the top half of the screen ...
  if (topofscreen < nitems - list_height &&
      ncursor >= topofscreen + list_height - 3) topofscreen++;
  // move cursor if there are any more ...
  if (cursorline+1 < nitems) ncursor++;
  setcursor(ncursor);
}

void baselist::kd_up() {
  int ncursor= cursorline;
  // scroll by one line if there are any lines not shown yet
  // and we're not in the bottom half the screen ...
  if (topofscreen > 0 &&
      ncursor <= topofscreen + 2) topofscreen--;
  // move cursor if there are any to move it to ...
  if (cursorline > 0) ncursor--;
  setcursor(ncursor);
}

void baselist::kd_iscrollon() {
  infotopofscreen += info_height;
  if (infotopofscreen > infolines - info_height)
    infotopofscreen= infolines - info_height;
  if (infotopofscreen < 0) infotopofscreen= 0;
  refreshinfo();
}

void baselist::kd_iscrollback() {
  infotopofscreen -= info_height;
  if (infotopofscreen < 0) infotopofscreen= 0;
  refreshinfo();
}

void baselist::kd_iscrollon1() {
  if (infotopofscreen >= infolines - info_height) return;
  infotopofscreen++; refreshinfo();
}

void baselist::kd_iscrollback1() {
  if (infotopofscreen <= 0) return;
  infotopofscreen--; refreshinfo();
}

void baselist::kd_panon() {
  leftofscreen += xmax/3;
  if (leftofscreen > total_width - xmax) leftofscreen= total_width - xmax;
  if (leftofscreen < 0) leftofscreen= 0;
  refreshcolheads(); refreshlist(); redrawthisstate(); refreshinfo();
}

void baselist::kd_panback() {
  leftofscreen -= xmax/3;
  if (leftofscreen < 0) leftofscreen= 0;
  refreshcolheads(); refreshlist(); redrawthisstate(); refreshinfo();
}

void baselist::kd_panon1() {
  leftofscreen++;
  if (leftofscreen > total_width - xmax) leftofscreen= total_width - xmax;
  if (leftofscreen < 0) leftofscreen= 0;
  refreshcolheads(); refreshlist(); redrawthisstate(); refreshinfo();
}

void baselist::kd_panback1() {
  leftofscreen--;
  if (leftofscreen < 0) leftofscreen= 0;
  refreshcolheads(); refreshlist(); redrawthisstate(); refreshinfo();
}
