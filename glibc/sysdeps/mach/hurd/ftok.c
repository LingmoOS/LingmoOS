/* Copyright (C) 1995, 1996, 2000, 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sys/ipc.h>
#include <sys/stat.h>


/* In the Hurd, we use the second-to-most-significant bit as flag for
   private keys.  We use a different order of the components so that
   the biggest one---the inode number---is affected by this.  */

key_t
ftok (const char *pathname, int proj_id)
{
  struct stat64 st;
  key_t key;

  if (__stat64 (pathname, &st) < 0)
    return (key_t) -1;

  key = ((st.st_dev & 0xff) | ((proj_id & 0xff) << 8)
	 | ((st.st_ino & 0x3fff) << 16));

  return key;
}
