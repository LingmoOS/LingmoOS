/*
 * dpkg - main program for package management
 * errors.c - per package error handling
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2007-2014 Guillem Jover <guillem@debian.org>
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
#include <sys/stat.h>
#include <sys/wait.h>

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/options.h>

#include "main.h"

bool abort_processing = false;

static int nerrs = 0;

struct error_report {
  struct error_report *next;
  char *what;
};

static struct error_report *reports = NULL;
static struct error_report **lastreport= &reports;
static struct error_report emergency;

static void
enqueue_error_report(const char *arg)
{
  struct error_report *nr;

  nr = malloc(sizeof(*nr));
  if (!nr) {
    notice(_("failed to allocate memory for new entry in list of failed packages: %s"),
           strerror(errno));
    abort_processing = true;
    nr= &emergency;
  }
  nr->what = m_strdup(arg);
  nr->next = NULL;
  *lastreport= nr;
  lastreport= &nr->next;

  if (++nerrs < errabort)
    return;
  notice(_("too many errors, stopping"));
  abort_processing = true;
}

void
print_error_perpackage(const char *emsg, const void *data)
{
  const char *pkgname = data;

  notice(_("error processing package %s (--%s):\n %s"),
         pkgname, cipaction->olong, emsg);

  statusfd_send("status: %s : %s : %s", pkgname, "error", emsg);

  enqueue_error_report(pkgname);
}

void
print_error_perarchive(const char *emsg, const void *data)
{
  const char *filename = data;

  notice(_("error processing archive %s (--%s):\n %s"),
         filename, cipaction->olong, emsg);

  statusfd_send("status: %s : %s : %s", filename, "error", emsg);

  enqueue_error_report(filename);
}

int
reportbroken_retexitstatus(int ret)
{
  if (reports) {
    fputs(_("Errors were encountered while processing:\n"),stderr);
    while (reports) {
      fprintf(stderr," %s\n",reports->what);
      free(reports->what);
      reports= reports->next;
    }
  }
  if (abort_processing) {
    fputs(_("Processing was halted because there were too many errors.\n"),stderr);
  }
  return nerrs ? 1 : ret;
}

bool
skip_due_to_hold(struct pkginfo *pkg)
{
  if (pkg->want != PKG_WANT_HOLD)
    return false;
  if (in_force(FORCE_HOLD)) {
    notice(_("package %s was on hold, processing it anyway as you requested"),
           pkg_name(pkg, pnaw_nonambig));
    return false;
  }
  printf(_("Package %s is on hold, not touching it.  Use --force-hold to override.\n"),
         pkg_name(pkg, pnaw_nonambig));
  return true;
}

