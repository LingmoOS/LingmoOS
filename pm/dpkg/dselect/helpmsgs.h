/*
 * dselect - Debian package maintenance user interface
 * helpmsgs.h - external definitions for the list of help messages
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

#ifndef HELPMSGS_H
#define HELPMSGS_H

struct helpmessage {
  const char *title;
  const char *text;
};

extern const struct helpmessage hlp_listkeys;
extern const struct helpmessage hlp_mainintro;
extern const struct helpmessage hlp_readonlyintro;
extern const struct helpmessage hlp_recurintro;
extern const struct helpmessage hlp_displayexplain1;
extern const struct helpmessage hlp_displayexplain2;
extern const struct helpmessage hlp_methintro;
extern const struct helpmessage hlp_methkeys;

#endif /* HELPMSGS_H */
