/*
 * dpkg-deb - construction and deconstruction of *.deb archives
 * main.c - main program
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2006-2014 Guillem Jover <guillem@debian.org>
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
#include <sys/wait.h>

#include <limits.h>
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif
#include <errno.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <dpkg/macros.h>
#include <dpkg/i18n.h>
#include <dpkg/dpkg.h>
#include <dpkg/dpkg-db.h>
#include <dpkg/compress.h>
#include <dpkg/options.h>

#include "dpkg-deb.h"

const char *opt_showformat = "${Package}\t${Version}\n";

static int
printversion(const char *const *argv)
{
  printf(_("Debian '%s' package archive backend version %s.\n"),
         BACKEND, PACKAGE_RELEASE);
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
"Usage: %s [<option>...] <command>\n"
"\n"), BACKEND);

  printf(_(
"Commands:\n"
"  -b|--build <directory> [<deb>]   Build an archive.\n"
"  -c|--contents <deb>              List contents.\n"
"  -I|--info <deb> [<cfile>...]     Show info to stdout.\n"
"  -W|--show <deb>                  Show information on package(s)\n"
"  -f|--field <deb> [<cfield>...]   Show field(s) to stdout.\n"
"  -e|--control <deb> [<directory>] Extract control info.\n"
"  -x|--extract <deb> <directory>   Extract files.\n"
"  -X|--vextract <deb> <directory>  Extract & list files.\n"
"  -R|--raw-extract <deb> <directory>\n"
"                                   Extract control info and files.\n"
"  --ctrl-tarfile <deb>             Output control tarfile.\n"
"  --fsys-tarfile <deb>             Output filesystem tarfile.\n"
"\n"));

  printf(_(
"  -?, --help                       Show this help message.\n"
"      --version                    Show the version.\n"
"\n"));

  printf(_(
"<deb> is the filename of a Debian format archive.\n"
"<cfile> is the name of an administrative file component.\n"
"<cfield> is the name of a field in the main 'control' file.\n"
"\n"));

  printf(_(
"Options:\n"
"  -v, --verbose                    Enable verbose output.\n"
"  -D, --debug                      Enable debugging output.\n"
"      --showformat=<format>        Use alternative format for --show.\n"
"      --deb-format=<format>        Select archive format.\n"
"                                     Allowed values: 0.939000, 2.0 (default).\n"
"      --no-check                   Suppress all checks (build bad packages).\n"
"      --nocheck                    Alias for --no-check.\n"
"      --root-owner-group           Forces the owner and groups to root.\n"
"      --threads-max=<threads>      Use at most <threads> with compressor.\n"
"      --[no-]uniform-compression   Use the compression params on all members.\n"
"  -Z, --compression=<compressor>   Set build compression type.\n"
"                                     Allowed types: gzip, xz, zstd, none.\n"
"  -z, --compression-level=<level>  Set build compression level.\n"
"  -S, --compression-strategy=<name>\n"
"                                   Set build compression strategy.\n"
"                                     Allowed values: none; extreme (xz);\n"
"                                     filtered, huffman, rle, fixed (gzip).\n"
"\n"));

  printf(_(
"Format syntax:\n"
"  A format is a string that will be output for each package. The format\n"
"  can include the standard escape sequences \\n (newline), \\r (carriage\n"
"  return) or \\\\ (plain backslash). Package information can be included\n"
"  by inserting variable references to package fields using the ${var[;width]}\n"
"  syntax. Fields will be right-aligned unless the width is negative in which\n"
"  case left alignment will be used.\n"));

  printf(_(
"\n"
"Use 'dpkg' to install and remove packages from your system, or\n"
"'apt' or 'aptitude' for user-friendly package management. Packages\n"
"unpacked using 'dpkg-deb --extract' will be incorrectly installed !\n"));

  m_output(stdout, _("<standard output>"));

  return 0;
}

static const char printforhelp[] =
  N_("Type dpkg-deb --help for help about manipulating *.deb files;\n"
     "Type dpkg --help for help about installing and deinstalling packages.");

int opt_debug = 0;
int opt_check = 1;
int opt_verbose = 0;
int opt_root_owner_group = 0;
int opt_uniform_compression = 1;

struct deb_version deb_format = DEB_VERSION(2, 0);

static void
set_deb_format(const struct cmdinfo *cip, const char *value)
{
  const char *err;

  err = deb_version_parse(&deb_format, value);
  if (err)
    badusage(_("invalid deb format version: %s"), err);

  if ((deb_format.major == 2 && deb_format.minor == 0) ||
      (deb_format.major == 0 && deb_format.minor == 939000))
    return;
  else
    badusage(_("unknown deb format version: %s"), value);
}

struct compress_params compress_params_deb0 = {
  .type = COMPRESSOR_TYPE_GZIP,
  .strategy = COMPRESSOR_STRATEGY_NONE,
  .level = -1,
  .threads_max = -1,
};

struct compress_params compress_params = {
  .type = DEB_DEFAULT_COMPRESSOR,
  .strategy = COMPRESSOR_STRATEGY_NONE,
  .level = -1,
  .threads_max = -1,
};

static long
parse_compress_level(const char *str)
{
  long value;
  char *end;

  errno = 0;
  value = strtol(str, &end, 10);
  if (str == end || *end != '\0' || errno != 0)
    return 0;

  return value;
}

static void
set_compress_level(const struct cmdinfo *cip, const char *value)
{
  compress_params.level = dpkg_options_parse_arg_int(cip, value);
}

static void
set_compress_strategy(const struct cmdinfo *cip, const char *value)
{
  compress_params.strategy = compressor_get_strategy(value);
  if (compress_params.strategy == COMPRESSOR_STRATEGY_UNKNOWN)
    badusage(_("unknown compression strategy '%s'!"), value);
}

static enum compressor_type
parse_compress_type(const char *value)
{
  enum compressor_type type;

  type = compressor_find_by_name(value);
  if (type == COMPRESSOR_TYPE_UNKNOWN)
    badusage(_("unknown compression type '%s'!"), value);
  if (type == COMPRESSOR_TYPE_LZMA)
    badusage(_("obsolete compression type '%s'; use xz instead"), value);
  if (type == COMPRESSOR_TYPE_BZIP2)
    badusage(_("obsolete compression type '%s'; use xz or gzip instead"), value);

  return type;
}

static void
set_compress_type(const struct cmdinfo *cip, const char *value)
{
  compress_params.type = parse_compress_type(value);
}

static long
parse_threads_max(const char *str)
{
  long value;
  char *end;

  errno = 0;
  value = strtol(str, &end, 10);
  if (str == end || *end != '\0' || errno != 0)
    return 0;

  return value;
}

static void
set_threads_max(const struct cmdinfo *cip, const char *value)
{
  compress_params.threads_max = dpkg_options_parse_arg_int(cip, value);
}

static const struct cmdinfo cmdinfos[]= {
  ACTION("build",         'b', 0, do_build),
  ACTION("contents",      'c', 0, do_contents),
  ACTION("control",       'e', 0, do_control),
  ACTION("info",          'I', 0, do_info),
  ACTION("field",         'f', 0, do_field),
  ACTION("extract",       'x', 0, do_extract),
  ACTION("vextract",      'X', 0, do_vextract),
  ACTION("raw-extract",   'R', 0, do_raw_extract),
  ACTION("ctrl-tarfile",  0,   0, do_ctrltarfile),
  ACTION("fsys-tarfile",  0,   0, do_fsystarfile),
  ACTION("show",          'W', 0, do_showinfo),
  ACTION("help",          '?', 0, usage),
  ACTION("version",       0,   0, printversion),

  { "deb-format",    0,   1, NULL,           NULL,         set_deb_format   },
  { "debug",         'D', 0, &opt_debug,     NULL,         NULL,          1 },
  { "verbose",       'v', 0, &opt_verbose,   NULL,         NULL,          1 },
  { "nocheck",       0,   0, &opt_check,     NULL,         NULL,          0 },
  { "no-check",      0,   0, &opt_check,     NULL,         NULL,          0 },
  { "root-owner-group",    0, 0, &opt_root_owner_group,    NULL, NULL,    1 },
  { "threads-max",   0,   1, NULL,           NULL,         set_threads_max  },
  { "uniform-compression", 0, 0, &opt_uniform_compression, NULL, NULL,    1 },
  { "no-uniform-compression", 0, 0, &opt_uniform_compression, NULL, NULL, 0 },
  { "compression",           'Z', 1, NULL,   NULL,         set_compress_type  },
  { "compression-level",     'z', 1, NULL,   NULL,         set_compress_level },
  { "compression-strategy",  'S', 1, NULL,   NULL,         set_compress_strategy },
  { "showformat",    0,   1, NULL,           &opt_showformat,  NULL         },
  {  NULL,           0,   0, NULL,           NULL,         NULL             }
};

int main(int argc, const char *const *argv) {
  struct dpkg_error err;
  char *env;
  int ret;

  dpkg_locales_init(PACKAGE);
  dpkg_program_init(BACKEND);
  /* XXX: Integrate this into options initialization/parsing. */
  env = getenv("DPKG_DEB_THREADS_MAX");
  if (str_is_set(env))
    compress_params.threads_max = parse_threads_max(env);
  env = getenv("DPKG_DEB_COMPRESSOR_TYPE");
  if (str_is_set(env))
    compress_params.type = parse_compress_type(env);
  env = getenv("DPKG_DEB_COMPRESSOR_LEVEL");
  if (str_is_set(env))
    compress_params.level = parse_compress_level(env);
  dpkg_options_parse(&argv, cmdinfos, printforhelp);

  if (!cipaction) badusage(_("need an action option"));

  if (!opt_uniform_compression && deb_format.major == 0)
    badusage(_("unsupported deb format '%d.%d' with non-uniform compression"),
             deb_format.major, deb_format.minor);

  if (deb_format.major == 0)
    compress_params = compress_params_deb0;

  if (!compressor_check_params(&compress_params, &err))
    badusage(_("invalid compressor parameters: %s"), err.str);

  if (opt_uniform_compression &&
      (compress_params.type != COMPRESSOR_TYPE_NONE &&
       compress_params.type != COMPRESSOR_TYPE_GZIP &&
       compress_params.type != COMPRESSOR_TYPE_ZSTD &&
       compress_params.type != COMPRESSOR_TYPE_XZ))
    badusage(_("unsupported compression type '%s' with uniform compression"),
             compressor_get_name(compress_params.type));

  ret = cipaction->action(argv);

  dpkg_program_done();
  dpkg_locales_done();

  return ret;
}
