/*
 * dpkg - main program for package management
 * update.c - options which update the ‘available’ database
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

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/options.h>

#include "main.h"

int
updateavailable(const char *const *argv)
{
  const char *sourcefile= argv[0];
  char *availfile;
  int count= 0;

  modstatdb_init();

  switch (cipaction->arg_int) {
  case act_avclear:
    if (sourcefile) badusage(_("--%s takes no arguments"),cipaction->olong);
    break;
  case act_avreplace: case act_avmerge:
    if (sourcefile == NULL)
      sourcefile = "-";
    else if (argv[1])
      badusage(_("--%s takes at most one Packages-file argument"),
               cipaction->olong);
    break;
  default:
    internerr("unknown action '%d'", cipaction->arg_int);
  }

  if (f_act) {
    const char *dbdir = dpkg_db_get_dir();

    if (access(dbdir, W_OK)) {
      if (errno != EACCES)
        ohshite(_("unable to access dpkg database directory '%s' for bulk available update"),
                dbdir);
      else
        ohshit(_("required write access to dpkg database directory '%s' for bulk available update"),
               dbdir);
    }
    modstatdb_lock();
  }

  switch (cipaction->arg_int) {
  case act_avreplace:
    printf(_("Replacing available packages info, using %s.\n"),sourcefile);
    break;
  case act_avmerge:
    printf(_("Updating available packages info, using %s.\n"),sourcefile);
    break;
  case act_avclear:
    break;
  default:
    internerr("unknown action '%d'", cipaction->arg_int);
  }

  availfile = dpkg_db_get_path(AVAILFILE);

  if (cipaction->arg_int == act_avmerge)
    parsedb(availfile, pdb_parse_available, NULL);

  if (cipaction->arg_int != act_avclear)
    count += parsedb(sourcefile,
                     pdb_parse_available | pdb_ignoreolder | pdb_dash_is_stdin,
                     NULL);

  if (f_act) {
    writedb(availfile, wdb_dump_available);
    modstatdb_unlock();
  }

  free(availfile);

  if (cipaction->arg_int != act_avclear)
    printf(P_("Information about %d package was updated.\n",
              "Information about %d packages was updated.\n", count), count);

  modstatdb_done();

  return 0;
}

int
forgetold(const char *const *argv)
{
  if (*argv)
    badusage(_("--%s takes no arguments"), cipaction->olong);

  warning(_("obsolete '--%s' option; unavailable packages are automatically cleaned up"),
          cipaction->olong);

  return 0;
}
