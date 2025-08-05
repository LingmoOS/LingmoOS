/*
 * dpkg-split - splitting and joining of multipart *.deb archives
 * dpkg-split.h - external definitions for this program
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2012 Guillem Jover <guillem@debian.org>
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

#ifndef DPKG_SPLIT_H
#define DPKG_SPLIT_H

#include <dpkg/ar.h>
#include <dpkg/deb-version.h>

action_func do_split;
action_func do_join;
action_func do_info;
action_func do_auto;
action_func do_queue;
action_func do_discard;

struct partinfo {
  struct deb_version fmtversion;
  const char *filename;
  const char *package;
  const char *version;
  const char *arch;
  const char *md5sum;
  off_t orglength;
  int thispartn, maxpartn;
  off_t maxpartlen;
  off_t thispartoffset;
  off_t thispartlen;
  /** Size of header in part file. */
  off_t headerlen;
  off_t filesize;
};

struct partqueue {
  struct partqueue *nextinqueue;

  /** Only fields filename, md5sum, maxpartlen, thispartn, maxpartn
   * are valid; the rest are NULL. If the file is not named correctly
   * to be a part file md5sum is NULL too and the numbers are zero. */
  struct partinfo info;
};

extern off_t opt_maxpartsize;
extern const char *opt_depotdir;
extern const char *opt_outputfile;
extern int opt_npquiet;
extern int opt_msdos;

void read_fail(int rc, const char *filename, const char *what) DPKG_ATTR_NORET;
void print_info(const struct partinfo *pi);
struct partinfo *read_info(struct dpkg_ar *ar, struct partinfo *ir);

void reassemble(struct partinfo **partlist, const char *outputfile);
void mustgetpartinfo(const char *filename, struct partinfo *ri);
void addtopartlist(struct partinfo**, struct partinfo*, struct partinfo *refi);

#define SPLITVERSION       "2.1"

#define PARTSDIR          "parts"

#define PARTMAGIC         "debian-split"
#define HEADERALLOWANCE    1024

#define SPLITPARTDEFMAX    (450 * 1024)

#endif /* DPKG_SPLIT_H */
