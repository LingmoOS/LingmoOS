/*
 * dpkg-split - splitting and joining of multipart *.deb archives
 * main.c - main program
 *
 * Copyright © 1994-1996 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006-2012 Guillem Jover <guillem@debian.org>
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

#include <errno.h>
#include <limits.h>
#include <inttypes.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/macros.h>
#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/debug.h>
#include <dpkg/fsys.h>
#include <dpkg/options.h>

#include "dpkg-split.h"

static int
printversion(const char *const *argv)
{
  printf(_("Debian '%s' package split/join tool; version %s.\n"),
         SPLITTER, PACKAGE_RELEASE);

  printf(_(
"This is free software; see the GNU General Public License version 2 or\n"
"later for copying conditions. There is NO warranty.\n"));

  m_output(stdout, _("<standard output>"));

  return 0;
}

static int
usage(const char *const *argv)
{
  printf(_(
"Usage: %s [<option> ...] <command>\n"
"\n"), SPLITTER);

  printf(_(
"Commands:\n"
"  -s|--split <file> [<prefix>]     Split an archive.\n"
"  -j|--join <part> <part> ...      Join parts together.\n"
"  -I|--info <part> ...             Display info about a part.\n"
"  -a|--auto -o <complete> <part>   Auto-accumulate parts.\n"
"  -l|--listq                       List unmatched pieces.\n"
"  -d|--discard [<filename> ...]    Discard unmatched pieces.\n"
"\n"));

  printf(_(
"  -?, --help                       Show this help message.\n"
"      --version                    Show the version.\n"
"\n"));

  printf(_(
"Options:\n"
"      --depotdir <directory>       Use <directory> instead of %s/%s.\n"
"      --admindir <directory>       Use <directory> instead of %s.\n"
"      --root <directory>           Use <directory> instead of %s.\n"
"  -S, --partsize <size>            In KiB, for -s (default is 450).\n"
"  -o, --output <file>              Filename, for -j (default is\n"
"                                     <package>_<version>_<arch>.deb).\n"
"  -Q, --npquiet                    Be quiet when -a is not a part.\n"
"      --msdos                      Generate 8.3 filenames.\n"
"\n"), ADMINDIR, PARTSDIR, ADMINDIR, "/");

  printf(_(
"Exit status:\n"
"  0 = ok\n"
"  1 = with --auto, file is not a part\n"
"  2 = trouble\n"));


  m_output(stdout, _("<standard output>"));

  return 0;
}

static const char printforhelp[] = N_("Type dpkg-split --help for help.");

off_t opt_maxpartsize = SPLITPARTDEFMAX;
const char *opt_depotdir;
const char *opt_outputfile = NULL;
int opt_npquiet = 0;
int opt_msdos = 0;

void DPKG_ATTR_NORET
read_fail(int rc, const char *filename, const char *what)
{
  if (rc >= 0)
    ohshit(_("unexpected end of file in %s in %.255s"), what, filename);
  else
    ohshite(_("error reading %s from file %.255s"), what, filename);
}

static void
set_part_size(const struct cmdinfo *cip, const char *value)
{
  off_t newpartsize;
  char *endp;

  errno = 0;
  newpartsize = strtoimax(value, &endp, 10);
  if (value == endp || *endp)
    badusage(_("invalid integer for --%s: '%.250s'"), cip->olong, value);
  if (newpartsize <= 0 || newpartsize > (INT_MAX >> 10) || errno == ERANGE)
    badusage(_("part size is far too large or is not positive"));

  opt_maxpartsize = newpartsize << 10;
  if (opt_maxpartsize <= HEADERALLOWANCE)
    badusage(_("part size must be at least %d KiB (to allow for header)"),
             (HEADERALLOWANCE >> 10) + 1);
}

static const struct cmdinfo cmdinfos[]= {
  ACTION("split",   's',  0,  do_split),
  ACTION("join",    'j',  0,  do_join),
  ACTION("info",    'I',  0,  do_info),
  ACTION("auto",    'a',  0,  do_auto),
  ACTION("listq",   'l',  0,  do_queue),
  ACTION("discard", 'd',  0,  do_discard),
  ACTION("help",    '?',  0,  usage),
  ACTION("version",  0,   0,  printversion),

  { "admindir",      0,   1,  NULL, NULL,             set_admindir,   0   },
  { "root",          0,   1,  NULL, NULL,             set_root,       0   },
  { "depotdir",      0,   1,  NULL, &opt_depotdir,    NULL                },
  { "partsize",     'S',  1,  NULL, NULL,             set_part_size       },
  { "output",       'o',  1,  NULL, &opt_outputfile,  NULL                },
  { "npquiet",      'Q',  0,  &opt_npquiet, NULL,     NULL,           1   },
  { "msdos",         0,   0,  &opt_msdos, NULL,       NULL,           1   },
  {  NULL,              0                                              }
};

int main(int argc, const char *const *argv) {
  int ret;

  dpkg_locales_init(PACKAGE);
  dpkg_program_init(SPLITTER);
  dpkg_options_parse(&argv, cmdinfos, printforhelp);

  debug(dbg_general, "root=%s admindir=%s", dpkg_fsys_get_dir(), dpkg_db_get_dir());

  if (opt_depotdir == NULL)
    opt_depotdir = dpkg_db_get_path(PARTSDIR);

  if (!cipaction) badusage(_("need an action option"));

  ret = cipaction->action(argv);

  m_output(stderr, _("<standard error>"));

  dpkg_program_done();
  dpkg_locales_done();

  return ret;
}
