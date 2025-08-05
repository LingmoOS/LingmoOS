/*
 * dselect - Debian package maintenance user interface
 * basetop.cc - base list class redraw of top
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
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

void baselist::refreshlist() {
  redrawitemsrange(topofscreen, min(nitems, topofscreen + list_height));
  int y, x, maxy, maxx;
  y = min(list_row + list_height - 1, list_row + nitems - topofscreen - 1);
  x = min(total_width - leftofscreen - 1, xmax - 1);
  pnoutrefresh(listpad, 0, leftofscreen, list_row, 0, y, x);
  getmaxyx(listpad,maxy,maxx);
  y++;
  while (y < list_row + list_height - 1) {
    pnoutrefresh(listpad, maxy-1,leftofscreen, y,0, y,x);
    y++;
  }
}

void baselist::redrawitemsrange(int start, int end) {
  int i;
  for (i = start; i < end; i++) {
    redraw1item(i);
  }
}

void baselist::refreshcolheads() {
  pnoutrefresh(colheadspad, 0,leftofscreen, colheads_row,0,
               colheads_row, min(total_width - leftofscreen - 1, xmax - 1));
}
