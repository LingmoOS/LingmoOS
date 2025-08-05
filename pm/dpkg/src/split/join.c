/*
 * dpkg-split - splitting and joining of multipart *.deb archives
 * join.c - joining
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

#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/buffer.h>
#include <dpkg/options.h>

#include "dpkg-split.h"

void reassemble(struct partinfo **partlist, const char *outputfile) {
  struct dpkg_error err;
  int fd_out;
  int i;

  printf(P_("Putting package %s together from %d part: ",
            "Putting package %s together from %d parts: ",
            partlist[0]->maxpartn),
         partlist[0]->package,partlist[0]->maxpartn);

  fd_out = creat(outputfile, 0644);
  if (fd_out < 0)
    ohshite(_("unable to open output file '%.250s'"), outputfile);
  for (i=0; i<partlist[0]->maxpartn; i++) {
    struct partinfo *pi = partlist[i];
    int fd_in;

    fd_in = open(pi->filename, O_RDONLY);
    if (fd_in < 0)
      ohshite(_("unable to (re)open input part file '%.250s'"), pi->filename);
    if (fd_skip(fd_in, pi->headerlen, &err) < 0)
      ohshit(_("cannot skip split package header for '%s': %s"), pi->filename,
             err.str);
    if (fd_fd_copy(fd_in, fd_out, pi->thispartlen, &err) < 0)
      ohshit(_("cannot append split package part '%s' to '%s': %s"),
             pi->filename, outputfile, err.str);
    close(fd_in);

    printf("%d ", i + 1);
  }
  if (fsync(fd_out))
    ohshite(_("unable to sync file '%s'"), outputfile);
  if (close(fd_out))
    ohshite(_("unable to close file '%s'"), outputfile);

  printf(_("done\n"));
}


void addtopartlist(struct partinfo **partlist,
                   struct partinfo *pi, struct partinfo *refi) {
  int i;

  if (strcmp(pi->package,refi->package) ||
      strcmp(pi->version,refi->version) ||
      strcmp(pi->md5sum,refi->md5sum) ||
      pi->orglength != refi->orglength ||
      pi->maxpartn != refi->maxpartn ||
      pi->maxpartlen != refi->maxpartlen) {
    print_info(pi);
    print_info(refi);
    ohshit(_("files '%.250s' and '%.250s' are not parts of the same file"),
           pi->filename,refi->filename);
  }
  i= pi->thispartn-1;
  if (partlist[i])
    ohshit(_("there are several versions of part %d - at least '%.250s' and '%.250s'"),
           pi->thispartn, pi->filename, partlist[i]->filename);
  partlist[i]= pi;
}

int
do_join(const char *const *argv)
{
  const char *thisarg;
  struct partqueue *queue = NULL;
  struct partqueue *pq;
  struct partinfo *refi, **partlist;
  int i;

  if (!*argv)
    badusage(_("--%s requires one or more part file arguments"),
             cipaction->olong);
  while ((thisarg= *argv++)) {
    pq = nfmalloc(sizeof(*pq));

    mustgetpartinfo(thisarg,&pq->info);

    pq->nextinqueue= queue;
    queue= pq;
  }
  refi= NULL;
  for (pq= queue; pq; pq= pq->nextinqueue)
    if (!refi || pq->info.thispartn < refi->thispartn) refi= &pq->info;
  if (refi == NULL)
    internerr("empty deb part queue");

  partlist = nfmalloc(sizeof(*partlist) * refi->maxpartn);
  for (i = 0; i < refi->maxpartn; i++)
    partlist[i] = NULL;
  for (pq= queue; pq; pq= pq->nextinqueue) {
    struct partinfo *pi = &pq->info;

    addtopartlist(partlist,pi,refi);
  }
  for (i=0; i<refi->maxpartn; i++) {
    if (!partlist[i]) ohshit(_("part %d is missing"),i+1);
  }
  if (!opt_outputfile) {
    char *p;

    p= nfmalloc(strlen(refi->package)+1+strlen(refi->version)+sizeof(DEBEXT));
    strcpy(p,refi->package);
    strcat(p, "_");
    strcat(p,refi->version);
    strcat(p, "_");
    strcat(p, refi->arch ? refi->arch : "unknown");
    strcat(p,DEBEXT);
    opt_outputfile = p;
  }
  reassemble(partlist, opt_outputfile);

  return 0;
}
