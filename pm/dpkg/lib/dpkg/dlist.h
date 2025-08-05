/*
 * dlist.h - macros for handling doubly linked lists
 *
 * Copyright © 1997-1999 Ian Jackson <ijackson@chiark.greenend.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef DPKG_DLIST_H
#define DPKG_DLIST_H

#define LIST_UNLINK_PART(list, node, part)		\
  do {							\
    if ((node)->part.prev)				\
      (node)->part.prev->part.next = (node)->part.next;	\
    else						\
      (list).head = (node)->part.next;			\
    if ((node)->part.next)				\
      (node)->part.next->part.prev = (node)->part.prev;	\
    else						\
      (list).tail = (node)->part.prev;			\
  } while (0)

#define LIST_LINK_TAIL_PART(list, node, part)		\
  do {							\
    (node)->part.next = NULL;				\
    (node)->part.prev = (list).tail;			\
    if ((list).tail)					\
      (list).tail->part.next = (node);			\
    else (list).head = (node);				\
      (list).tail = (node);				\
  } while (0)

#endif
