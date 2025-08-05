/*
 * libdpkg - Debian packaging suite library routines
 * options.c - option parsing functions
 *
 * Copyright © 1994,1995 Ian Jackson <ijackson@chiark.greenend.org.uk>
 * Copyright © 2000,2002 Wichert Akkerman <wichert@deephackmode.org>
 * Copyright © 2008-2015 Guillem Jover <guillem@debian.org>
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
#include <limits.h>
#include <string.h>
#include <dirent.h>
#include <stdarg.h>
#include <stdlib.h>

#include <dpkg/i18n.h>
#include <dpkg/c-ctype.h>
#include <dpkg/dpkg.h>
#include <dpkg/string.h>
#include <dpkg/options.h>

static const char *printforhelp;

void
badusage(const char *fmt, ...)
{
  char *buf = NULL;
  va_list args;

  va_start(args, fmt);
  m_vasprintf(&buf, fmt, args);
  va_end(args);

  ohshit("%s\n\n%s", buf, gettext(printforhelp));
}

static void DPKG_ATTR_NORET DPKG_ATTR_PRINTF(3)
config_error(const char *file_name, int line_num, const char *fmt, ...)
{
  char *buf = NULL;
  va_list args;

  va_start(args, fmt);
  m_vasprintf(&buf, fmt, args);
  va_end(args);

  ohshit(_("configuration error: %s:%d: %s"), file_name, line_num, buf);
}

static void
dpkg_options_load_file(const char *fn, const struct cmdinfo *cmdinfos)
{
  FILE *file;
  int line_num = 0;
  char linebuf[MAX_CONFIG_LINE];

  file= fopen(fn, "r");
  if (!file) {
    if (errno==ENOENT)
      return;
    warning(_("failed to open configuration file '%.255s' for reading: %s"),
            fn, strerror(errno));
    return;
  }

  while (fgets(linebuf, sizeof(linebuf), file)) {
    char *opt;
    const struct cmdinfo *cip;

    line_num++;

    str_rtrim_spaces(linebuf, linebuf + strlen(linebuf));

    if ((linebuf[0] == '#') || (linebuf[0] == '\0'))
      continue;
    for (opt = linebuf; c_isalnum(*opt) || *opt == '-'; opt++) ;
    if (*opt == '\0')
      opt=NULL;
    else {
      *opt++ = '\0';
      if (*opt=='=') opt++;
      while (c_isspace(*opt))
        opt++;

      opt = str_strip_quotes(opt);
      if (opt == NULL)
        config_error(fn, line_num, _("unbalanced quotes in '%s'"), linebuf);
    }

    for (cip=cmdinfos; cip->olong || cip->oshort; cip++) {
      int l;

      if (!cip->olong) continue;
      if (strcmp(cip->olong, linebuf) == 0)
        break;
      l=strlen(cip->olong);
      if ((cip->takesvalue==2) && (linebuf[l]=='-') &&
          !opt && strncmp(linebuf, cip->olong, l) == 0) {
	opt=linebuf+l+1;
	break;
      }
    }

    if (!cip->olong)
      config_error(fn, line_num, _("unknown option '%s'"), linebuf);

    if (cip->takesvalue) {
      if (!opt)
        config_error(fn, line_num, _("'%s' needs a value"), linebuf);
      if (cip->call) cip->call(cip,opt);
      else
        *cip->sassignto = m_strdup(opt);
    } else {
      if (opt)
        config_error(fn, line_num, _("'%s' does not take a value"), linebuf);
      if (cip->call) cip->call(cip,NULL);
      else
        *cip->iassignto = cip->arg_int;
    }
  }
  if (ferror(file))
    ohshite(_("read error in configuration file '%.255s'"), fn);
  if (fclose(file))
    ohshite(_("error closing configuration file '%.255s'"), fn);
}

static int
valid_config_filename(const struct dirent *dent)
{
  const char *c;

  if (dent->d_name[0] == '.')
    return 0;

  for (c = dent->d_name; *c; c++)
    if (!c_isalnum(*c) && *c != '_' && *c != '-')
      return 0;

  if (*c == '\0')
    return 1;
  else
    return 0;
}

static void
dpkg_options_load_dir(const char *prog, const struct cmdinfo *cmdinfos)
{
  char *dirname;
  struct dirent **dlist;
  int dlist_n, i;

  dirname = str_fmt("%s/%s.cfg.d", CONFIGDIR, prog);

  dlist_n = scandir(dirname, &dlist, valid_config_filename, alphasort);
  if (dlist_n < 0) {
    if (errno == ENOENT) {
      free(dirname);
      return;
    } else
      ohshite(_("error opening configuration directory '%s'"), dirname);
  }

  for (i = 0; i < dlist_n; i++) {
    char *filename;

    filename = str_fmt("%s/%s", dirname, dlist[i]->d_name);
    dpkg_options_load_file(filename, cmdinfos);

    free(dlist[i]);
    free(filename);
  }

  free(dirname);
  free(dlist);
}

void
dpkg_options_load(const char *prog, const struct cmdinfo *cmdinfos)
{
  char *home, *file;

  dpkg_options_load_dir(prog, cmdinfos);

  file = str_fmt("%s/%s.cfg", CONFIGDIR, prog);
  dpkg_options_load_file(file, cmdinfos);
  free(file);

  home = getenv("HOME");
  if (home != NULL) {
    file = str_fmt("%s/.%s.cfg", home, prog);
    dpkg_options_load_file(file, cmdinfos);
    free(file);
  }
}

void
dpkg_options_parse(const char *const **argvp, const struct cmdinfo *cmdinfos,
                   const char *help_str)
{
  const struct cmdinfo *cip;
  const char *p, *value;
  int l;

  if (**argvp == NULL)
    badusage(_("missing program name in argv[0]"));

  printforhelp = help_str;

  ++(*argvp);
  while ((p = **argvp) && p[0] == '-' && p[1] != '\0') {
    ++(*argvp);
    if (strcmp(p, "--") == 0)
      break;
    if (*++p == '-') {
      ++p; value=NULL;
      for (cip= cmdinfos;
           cip->olong || cip->oshort;
           cip++) {
        if (!cip->olong) continue;
        if (strcmp(p, cip->olong) == 0)
          break;
        l= strlen(cip->olong);
        if (strncmp(p, cip->olong, l) == 0 &&
            (p[l]== ((cip->takesvalue==2) ? '-' : '='))) { value=p+l+1; break; }
      }
      if (!cip->olong) badusage(_("unknown option --%s"),p);
      if (cip->takesvalue) {
        if (!value) {
          value= *(*argvp)++;
          if (!value) badusage(_("--%s option takes a value"),cip->olong);
        }
        if (cip->call) cip->call(cip,value);
        else *cip->sassignto= value;
      } else {
        if (value) badusage(_("--%s option does not take a value"),cip->olong);
        if (cip->call) cip->call(cip,NULL);
        else
          *cip->iassignto = cip->arg_int;
      }
    } else {
      while (*p) {
        for (cip= cmdinfos; (cip->olong || cip->oshort) && *p != cip->oshort; cip++);
        if (!cip->oshort) badusage(_("unknown option -%c"),*p);
        p++;
        if (cip->takesvalue) {
          if (!*p) {
            value= *(*argvp)++;
            if (!value) badusage(_("-%c option takes a value"),cip->oshort);
          } else {
            value= p; p="";
            if (*value == '=') value++;
          }
          if (cip->call) cip->call(cip,value);
          else *cip->sassignto= value;
        } else {
          if (*p == '=') badusage(_("-%c option does not take a value"),cip->oshort);
          if (cip->call) cip->call(cip,NULL);
          else
            *cip->iassignto = cip->arg_int;
        }
      }
    }
  }
}

long
dpkg_options_parse_arg_int(const struct cmdinfo *cmd, const char *str)
{
  long value;
  char *end;

  errno = 0;
  value = strtol(str, &end, 0);
  if (str == end || *end || value < 0 || value > INT_MAX || errno != 0) {
    if (cmd->olong)
      badusage(_("invalid integer for --%s: '%.250s'"), cmd->olong, str);
    else
      badusage(_("invalid integer for -%c: '%.250s'"), cmd->oshort, str);
  }

  return value;
}

void
setobsolete(const struct cmdinfo *cip, const char *value)
{
  warning(_("obsolete option '--%s'"), cip->olong);
}

const struct cmdinfo *cipaction = NULL;

/* XXX: This function is a hack. */
static inline int
option_short(int c)
{
  return c ? c : '\b';
}

void
setaction(const struct cmdinfo *cip, const char *value)
{
  if (cipaction && cip)
    badusage(_("conflicting actions -%c (--%s) and -%c (--%s)"),
             option_short(cip->oshort), cip->olong,
             option_short(cipaction->oshort), cipaction->olong);
  cipaction = cip;
  if (cip && cip->takesvalue == 2 && cip->sassignto)
    *cipaction->sassignto = value;
}
