/*
 * dpkg - main program for package management
 * archives.h - functions common to archives.c and unpack.c
 *
 * Copyright © 1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2008-2013, 2015 Guillem Jover <guillem@debian.org>
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

#ifndef ARCHIVES_H
#define ARCHIVES_H

#include <stdbool.h>

#include <dpkg/tarfn.h>

struct tarcontext {
  int backendpipe;
  struct pkginfo *pkg;
  /** A queue of fsys_namenode that have been extracted anew. */
  struct fsys_namenode_queue *newfiles_queue;
  /** Are all “Multi-arch: same” instances about to be in sync? */
  bool pkgset_getting_in_sync;
};

struct pkg_deconf_list {
  struct pkg_deconf_list *next;
  struct pkginfo *pkg;
  struct pkginfo *pkg_removal;
  enum pkgwant reason;
};

extern struct varbuf_state fname_state;
extern struct varbuf_state fnametmp_state;
extern struct varbuf_state fnamenew_state;
extern struct varbuf fnamevb;
extern struct varbuf fnametmpvb;
extern struct varbuf fnamenewvb;
extern struct pkg_deconf_list *deconfigure;

void clear_deconfigure_queue(void);
void enqueue_deconfigure(struct pkginfo *pkg, struct pkginfo *pkg_removal,
                         enum pkgwant reason);
void enqueue_conflictor(struct pkginfo *pkg);

void cu_pathname(int argc, void **argv);
void cu_cidir(int argc, void **argv);
void cu_fileslist(int argc, void **argv);
void cu_backendpipe(int argc, void **argv);

void cu_installnew(int argc, void **argv);

void cu_prermupgrade(int argc, void **argv);
void cu_prerminfavour(int argc, void **argv);
void cu_preinstverynew(int argc, void **argv);
void cu_preinstnew(int argc, void **argv);
void cu_preinstupgrade(int argc, void **argv);
void cu_postrmupgrade(int argc, void **argv);

void cu_prermdeconfigure(int argc, void **argv);
void ok_prermdeconfigure(int argc, void **argv);

void setupfnamevbs(const char *filename);

int
tarobject(struct tar_archive *tar, struct tar_entry *ti);
int
tarfileread(struct tar_archive *tar, char *buf, int len);
void
tar_deferred_extract(struct fsys_namenode_list *files, struct pkginfo *pkg);

struct fsys_namenode_list *
tar_fsys_namenode_queue_push(struct fsys_namenode_queue *queue,
                             struct fsys_namenode *namenode);

bool
filesavespackage(struct fsys_namenode_list *, struct pkginfo *,
                 struct pkginfo *pkgbeinginstalled);

void check_conflict(struct dependency *dep, struct pkginfo *pkg,
                    const char *pfilename);
void check_breaks(struct dependency *dep, struct pkginfo *pkg,
                  const char *pfilename);

extern int cleanup_pkg_failed, cleanup_conflictor_failed;

#endif /* ARCHIVES_H */
